#include <onix/onix.h>
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/console.h>
#include <onix/printk.h>

int magic = ONIX_MAGIC;
char message[] = "hello onix!";
char buf[1024];


void kernel_init()
{
   
    console_init();
    
    int cnt = 30;
    while(cnt--){
        printk("hello onix %d\n",cnt);
    }

    return;
}