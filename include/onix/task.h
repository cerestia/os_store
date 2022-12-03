#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16

typedef u32 target_t();

typedef enum task_state_t
{
    TASK_INIT,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_SLEEPING,
    TASK_WAITING,
    TASK_DIED,
} task_state_t;

typedef struct task_t
{
    u32 *stack;
    task_state_t state;
    u32 priority;
    u32 ticks;   //剩余时间片
    u32 jiffies; //上次执行全局时间片
    u32 name[TASK_NAME_LEN];
    u32 uid;
    u32 pde; //页目录物理地址
    struct bitmap_t *vmap;
    u32 magic;
} task_t;

typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

// void task_init();
task_t *running_task();
void schedule();

#endif