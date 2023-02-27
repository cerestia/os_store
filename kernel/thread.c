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
#include <onix/string.h>

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

extern void osh_main();

void test_recursion()
{
    char tmp[0x400];
    test_recursion();
}

static void user_init_thread()
{

    while (true)
    {
        u32 status;
        pid_t pid = fork();
        if (pid)
        {
            pid_t child = waitpid(pid, &status);
            printf("wait pid %d status %d %d\n", child, status, time());
        }
        else
        {
            osh_main();
        }
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
        sleep(10);
    }
}