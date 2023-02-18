#include <onix/bitmap.h>
#include <onix/string.h>
#include <onix/onix.h>
#include <onix/assert.h>
#include <onix/memory.h>

void bitmap_make(bitmap_t *map, char *bits, u32 length, u32 offset)
{
    map->bits = bits;
    map->length = length;
    map->offset = offset;
}

void bitmap_init(bitmap_t *map, char *bits, u32 length, u32 start)
{
    memset(bits, 0, length);
    bitmap_make(map, bits, length, start);
}

// 测试某一位是否为 1
bool bitmap_test(bitmap_t *map, idx_t index)
{
    assert(index >= map->offset);
    // 获取对应页表索引
    idx_t idx = index - map->offset;
    // 数组中的字节
    u32 bytes = idx / 8;
    u8 bits = idx % 8;
    assert(bytes < map->length);

    return (map->bits[bytes] & (1 << bits));
}

void bitmap_set(bitmap_t *map, idx_t index, bool value)
{
    // valut必须是二值
    assert(value == 0 || value == 1);
    assert(index >= map->offset);

    idx_t idx = index - map->offset;
    u32 bytes = idx / 8;
    u8 bits = idx % 8;
    if (value)
    {
        map->bits[bytes] |= (1 << bits);
    }
    else
    {
        // 置为 0
        map->bits[bytes] &= ~(1 << bits);
    }
}

int bitmap_scan(bitmap_t *map, u32 count)
{
    int start = EOF;
    u32 bits_left = map->length * 8;
    u32 next_bit = 0;
    u32 counter = 0;

    while (bits_left-- > 0)
    {
        if (!bitmap_test(map, map->offset + next_bit))
        {
            counter++;
        }
        else
        {
            counter = 0;
        }
        next_bit++;
        if (counter == count)
        {
            start = next_bit - count;
            break;
        }
    }
    if (start == EOF)
        return EOF;

    bits_left = counter;
    next_bit = start;
    while (bits_left--)
    {
        bitmap_set(map, map->offset + next_bit, true);
        next_bit++;
    }

    return start + map->offset;
}

#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
void memory_test()
{
    u32 *pages = (u32 *)(0x200000);
    u32 count = 0x6fe;
    for (size_t i = 0; i < count; i++)
    {
        pages[i] = alloc_kpage(1);
        LOGK("0x%x\n", i);
    }

    for (size_t i = 0; i < count; i++)
    {
        free_kpage(pages[i], 1);
    }
}