#include <onix/fs.h>
#include <onix/syscall.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/buffer.h>
#include <onix/arena.h>
#include <onix/string.h>
#include <onix/stdlib.h>

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
        inode->access_time = time();

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
    // TODO %
    inode->desc = &((inode_desc_t *)buf->data)[(inode->nr - 1) % BLOCK_INODES];

    inode->create_time = inode->desc->mtime;
    inode->access_time = time();

    return inode;
}

void iput(inode_t *inode)
{
    if (!inode)
        return;

    // TODO: need write... ?
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