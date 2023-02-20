#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/list.h>

#define KERNEL_USER 0
#define NORMAL_USER 1000

#define TASK_NAME_LEN 16

typedef void target_t();

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
    list_node_t node; // 任务阻塞节点
    task_state_t state;
    u32 priority;
    int ticks;   // 剩余时间片
    u32 jiffies; // 上次执行全局时间片
    char name[TASK_NAME_LEN];
    u32 uid;
    u32 gid; // 用户组 id
    pid_t pid;
    pid_t ppid;
    u32 pde; // 页目录物理地址
    struct bitmap_t *vmap;
    u32 brk;    // 程序堆内存最高地址
    int status; //
    pid_t waitpid;
    struct inode_t *ipwd;  // 进程当前目录 inode program work directory
    struct inode_t *iroot; // 进程根目录 inode
    u16 umask;             // 进程用户权限
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

typedef struct intr_frame_t
{
    u32 vector;

    u32 edi;
    u32 esi;
    u32 ebp;
    // 虽然 pushad 把 esp 也压入，但 esp 是不断变化的，所以会被 popad 忽略
    u32 esp_dummy;

    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;

    u32 vector0;

    u32 error;

    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
} intr_frame_t;

// void task_init();
task_t *running_task();
void schedule();

pid_t task_fork();
void task_exit();
pid_t task_waitpid(pid_t pid, int32 *status);

void task_yield();
void task_block(task_t *task, list_t *blist, task_state_t state);
void task_unblock(task_t *task);

void task_sleep(u32 ms);
void task_wakeup();

void task_to_user_mode(target_t target);

pid_t sys_getpid();
pid_t sys_getppid();
#endif