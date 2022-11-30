#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void set_alarm();
extern void memory_test();

char message[] = "hello onix!";
char buf[1024];

void kernel_init()
{

    interrupt_init();

    //task_init(); 
    //clock_init(); 
    //time_init();
    //rtc_init();    
    memory_test();
 
    asm volatile("sti");
    hang();
}