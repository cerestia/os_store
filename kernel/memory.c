#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>

#define LOGK(fmt, args...) DEBUGK(fmt,##args)

#define ZONE_VALID 1 //ards可用内存区域
#define ZONE_RESERVED 2 //args 不可以区域

#define IDX(addr) ((u32)addr>>12) //获取addr 页索引

typedef struct ards_t
{
    u64 base;
    u64 size;
    u32 type;
}_packed ards_t;

u32 memory_base = 0; //可用内存基地址，应该等于 1M
u32 memory_size = 0;
u32 total_pages = 0;
u32 free_pages = 0;

void memory_init(u32 magic,u32 addr){
    u32 count;
    ards_t* ptr;

    if(magic==ONIX_MAGIC){
        count = *(u32*)addr;
        ptr = (ards_t*)(addr+4);
        for(size_t i=0;i<count;i++,ptr++){
            LOGK("Memory base 0x%p size 0x%p type %d\n",
                (u32)ptr->base,(u32)ptr->size,(u32)ptr->type);
            if(ptr->type==ZONE_VALID && ptr->size >memory_size){
                memory_base = (u32)ptr->base;
                memory_size = (u32)ptr->size;
            }
        }
    }
    else{
        panic("memory init magic unknown 0x%p\n",magic);
    }


    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)memory_base);
    LOGK("Memory size 0x%p\n", (u32)memory_size);

    assert(memory_base == MEMORY_BASE); // 内存开始的位置为 1M
    assert((memory_size & 0xfff) == 0); // 要求按页对齐

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n", total_pages);
    LOGK("Free pages %d\n", free_pages);

}