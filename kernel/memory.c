#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/stdlib.h>
#include <onix/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt,##args)

#define ZONE_VALID 1 //ards可用内存区域
#define ZONE_RESERVED 2 //args 不可以区域

#define IDX(addr) ((u32)addr>>12) //获取addr 页索引
#define PAGE(idx) ((u32)idx<<12)
#define ASSERT_PAGE(addr) assert((addr&0xfff)==0)


typedef struct ards_t
{
    u64 base;
    u64 size;
    u32 type;
}_packed ards_t;

static u32 memory_base = 0; //可用内存基地址，应该等于 1M
static u32 memory_size = 0;
static u32 total_pages = 0;
static u32 free_pages = 0;

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

static u32 start_page = 0;
static u8* memory_map;
static u32 memory_map_pages;

void memory_map_init(){
    //initial physical memory array
    memory_map = (u8*) memory_base;

    memory_map_pages = div_round_up(total_pages,PAGE_SIZE);
    LOGK("memory map page count %d\n",memory_map_pages);

    free_pages -= memory_map_pages;
    memset((void*)memory_map,0,memory_map_pages*PAGE_SIZE);

    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for(size_t i=0;i<start_page;i++){
        memory_map[i] = i;
    }

    LOGK("total pages %d free pages %d\n",total_pages, free_pages);
}

static u32 get_one_page(){
    for(size_t i=start_page;i<total_pages;i++){
        if(!memory_map[i]){
            memory_map[i] = 1;
            free_pages--;
            assert(free_pages >= 0);
            u32 page = ((u32)i) << 12;
            LOGK("Get page 0x%p\n",page);
            return page;
        }
    }
    panic("Out of memory!");
}

static void put_page(u32 addr){
    ASSERT_PAGE(addr);
    u32 idx = IDX(addr);

    assert(idx >=start_page & idx<total_pages);
    //只有一个引用
    assert(memory_map[idx]>=1);

    memory_map[idx]--;

    if(!memory_map[idx]){
        free_pages++;
    }

    assert(free_pages >0 && free_pages <total_pages);
    LOGK("Put page 0x%p\n",addr);
}

u32 inline get_cr3(){
    asm volatile("movl %cr3,%eax\n");
}

// 设置 cr3 寄存器，参数是页目录的地址
void inline set_cr3(u32 pde){
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

static inline void enable_page(){
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n");
}

static void entry_init(page_entry_t* entry,u32 index){
    *(u32 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

#define KERNEL_PAGE_DIR 0x200000

// 内核页表
#define KERNEL_PAGE_ENTRY 0x201000

void mapping_init(){
    page_entry_t* pde = (page_entry_t*)KERNEL_PAGE_DIR;
    memset(pde,0,PAGE_SIZE);
    entry_init(&pde[0],IDX(KERNEL_PAGE_ENTRY));

    page_entry_t* pte = (page_entry_t*)KERNEL_PAGE_ENTRY;
    memset(pte,0,PAGE_SIZE);

    page_entry_t* entry;
    for(size_t tidx=0;tidx<1024;tidx++){
        entry = &pte[tidx];
        entry_init(entry,tidx);
        memory_map[tidx] = 1; //设置物理内存数组，该页被占用
    }

    BMB;

    // 设置 cr3 寄存器
    set_cr3((u32)pde);

    BMB;
    enable_page();
}

