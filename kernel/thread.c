#include "onix/interrupt.h"
#include "onix/syscall.h"
#include "onix/debug.h"
#include <onix/printk.h>
#include <onix/task.h>
#include <onix/stdio.h>
#include <onix/arena.h>
#include <onix/stdio.h>
#include <onix/stdlib.h>
#include <onix/fs.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        // LOGK("idle task...%d\n", counter++);
        asm volatile(
            "sti\n"   // 开中断
            "hlt\n"); // 关闭cpu
        yield();
    }
}
#include <onix/mutex.h>

extern u32 keyboard_read(char *buf, u32 count);

lock_t lock;

void test_recursion()
{
    char tmp[0x400];
    test_recursion();
}

static void user_init_thread()
{
    char buf[256];
    fd_t fd;
    int len = 0;
    fd = open("/hello.txt", O_RDWR, 0755);
    len = read(fd, buf, sizeof(buf));
    printf("hello.txt content: %s length %d\n", buf, len);
    close(fd);

    fd = open("/world.txt", O_CREAT | O_RDWR, 0755);
    len = write(fd, buf, len);
    close(fd);

    while (true)
    {
        char ch;
        read(stdin, &ch, 1);
        write(stdout, &ch, 1);
        sleep(1000);
    }
}

void init_thread()
{

    char temp[100]; // 为了给栈留空间

    task_to_user_mode(user_init_thread);
}

void test_thread()
{
    set_interrupt_state(true);

    while (true)
    {
        test();
        sleep(10);
    }
}