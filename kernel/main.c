#include <onix/debug.h>
#include <onix/types.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void set_alarm();
extern void memory_map_init();
extern void mapping_init();
extern void memory_test();
extern void task_init();
extern void set_interrupt_state();
extern void syscall_init();
extern void keyboard_init();
extern void tss_init();
extern void arena_init();

char message[] = "hello onix!";
char buf[1024];

void kernel_init()
{
    tss_init();
    memory_map_init();
    mapping_init();
    arena_init();

    interrupt_init();
    clock_init();
    keyboard_init();

    task_init();
    syscall_init();
    set_interrupt_state(true);
}