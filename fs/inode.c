#include <onix/fs.h>
#include <onix/syscall.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/buffer.h>
#include <onix/arena.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/stat.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define INODE_NR 64

static inode_t inode_table[INODE_NR];

static inode_t *get_free_inode()
{
    for (size_t i = 0; i < INODE_NR; i++)
    {
        inode_t *inode = &inode_table[i];
        if (inode->dev == EOF)
        {
            return inode;
        }
    }
    panic("no more inode!!!");
}

static void put_free_inode(inode_t *inode)
{
    assert(inode != inode_table);
    assert(inode->count == 0);
    inode->dev = EOF;
}

inode_t *get_root_inode()
{
    return inode_table;
}

static inline idx_t inode_block(super_block_t *sb, idx_t nr)
{
    // boot ,super ,inmaps, zmaps, inode index start from 1
    return 2 + sb->desc->imap_blocks + sb->desc->zmap_blocks + (nr - 1) / BLOCK_INODES;
}

static inode_t *find_inode(dev_t dev, idx_t nr)
{
    super_block_t *sb = get_super(dev);
    assert(sb);
    list_t *list = &sb->inode_list;

    for (list_node_t *node = list->head.next; node != &list->tail; node = node->next)
    {
        inode_t *inode = element_entry(inode_t, node, node);
        if (inode->nr == nr)
        {
            return inode;
        }
    }
    return NULL;
}

inode_t *iget(dev_t dev, idx_t nr)
{
    inode_t *inode = find_inode(dev, nr);
    if (inode)
    {
        inode->count++;
        inode->atime = time();

        return inode;
    }

    super_block_t *sb = get_super(dev);
    assert(sb);

    assert(nr <= sb->desc->inodes);

    inode = get_free_inode();
    inode->dev = dev;
    inode->nr = nr;
    inode->count = 1;

    list_push(&sb->inode_list, &inode->node);
    idx_t block = inode_block(sb, inode->nr);
    buffer_t *buf = bread(inode->dev, block);

    inode->buf = buf;

    // 将缓冲视为一个 inode 描述符数组，获取对应的指针；
    inode->desc = &((inode_desc_t *)buf->data)[(inode->nr - 1) % BLOCK_INODES];

    inode->ctime = inode->desc->mtime;
    inode->atime = time();

    return inode;
}

void iput(inode_t *inode)
{
    if (!inode)
        return;

    if (inode->buf->dirty)
    {
        bwrite(inode->buf);
    }

    inode->count--;
    if (inode->count)
    {
        return;
    }

    brelse(inode->buf);

    list_remove(&inode->node);

    put_free_inode(inode);
}

void inode_init()
{
    for (size_t i = 0; i < INODE_NR; i++)
    {
        inode_t *inode = &inode_table[i];
        inode->dev = EOF;
    }
}

int inode_read(inode_t *inode, char *buf, u32 len, off_t offset)
{
    assert(ISFILE(inode->desc->mode) || ISDIR(inode->desc->mode));

    // 如果偏移量超过文件大小，返回 EOF
    if (offset >= inode->desc->size)
    {
        return EOF;
    }

    // 开始读取的位置
    u32 begin = offset;

    u32 left = MIN(len, inode->desc->size - offset);

    while (left)
    {
        // 找到对应的文件，所在文件块
        idx_t nr = bmap(inode, offset / BLOCK_SIZE, false);
        assert(nr);

        // 读取文件块缓冲
        buffer_t *bf = bread(inode->dev, nr);

        // 文件块中的偏移量
        u32 start = offset % BLOCK_SIZE;

        // 本次需要读取的字节数
        u32 chars = MIN(BLOCK_SIZE - start, left);

        offset += chars;
        left -= chars;

        // 文件块中的指针
        char *ptr = bf->data + start;

        // 拷贝内容
        memcpy(buf, ptr, chars);

        // 更新缓存位置
        buf += chars;

        // 释放文件块缓冲
        brelse(bf);
    }

    // 更新访问时间
    inode->atime = time();

    // 返回读取数量
    return offset - begin;
}

int inode_write(inode_t *inode, char *buf, u32 len, off_t offset)
{
    assert(inode->desc->mode);

    // 开始的位置
    u32 begin = offset;

    // 剩余数量
    u32 left = len;

    while (left)
    {
        // 找到文件块，若不存在则创建
        idx_t nr = bmap(inode, offset / BLOCK_SIZE, true);
        assert(nr);

        // 将读入文件块
        buffer_t *bf = bread(inode->dev, nr);
        bf->dirty = true;

        // 块中的偏移量
        u32 start = offset % BLOCK_SIZE;
        // 文件块中的指针
        char *ptr = bf->data + start;

        // 读取的数量
        u32 chars = MIN(BLOCK_SIZE - start, left);

        // 更新偏移量
        offset += chars;

        // 更新剩余字节数
        left -= chars;

        // 如果偏移量大于文件大小，则更新
        if (offset > inode->desc->size)
        {
            inode->desc->size = offset;
            inode->buf->dirty = true;
        }

        // 拷贝内容
        memcpy(ptr, buf, chars);

        // 更新缓存偏移
        buf += chars;

        // 释放文件块
        brelse(bf);
    }

    // 更新修改时间
    inode->desc->mtime = inode->atime = time();

    // TODO: 写入磁盘 ？
    bwrite(inode->buf);

    // 返回写入大小
    return offset - begin;
}

static void inode_bfree(inode_t *inode, u16 *array, int index, int level)
{
    if (!array[index])
    {
        return;
    }

    if (!level)
    {
        bfree(inode->dev, array[index]);
        return;
    }

    buffer_t *buf = bread(inode->dev, array[index]);
    for (size_t i = 0; i < BLOCK_INDEXES; i++)
    {
        inode_bfree(inode, (u16 *)buf->data, i, level - 1);
    }
    brelse(buf);
    bfree(inode->dev, array[index]);
}

void inode_truncate(inode_t *inode)
{
    if (!ISFILE(inode->desc->mode) && !ISDIR(inode->desc->mode))
    {
        return;
    }

    // 释放直接块
    for (size_t i = 0; i < DIRECT_BLOCK; i++)
    {
        inode_bfree(inode, inode->desc->zone, i, 0);
        inode->desc->zone[i] = 0;
    }

    // 释放一级间接块
    inode_bfree(inode, inode->desc->zone, DIRECT_BLOCK, 1);
    inode->desc->zone[DIRECT_BLOCK] = 0;

    // 释放二级间接块
    inode_bfree(inode, inode->desc->zone, DIRECT_BLOCK + 1, 2);
    inode->desc->zone[DIRECT_BLOCK + 1] = 0;

    inode->desc->size = 0;
    inode->buf->dirty = true;
    inode->desc->mtime = time();
    bwrite(inode->buf);
}