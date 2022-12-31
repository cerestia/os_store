#ifndef ONIX_MUTEX_H
#define ONIX_MUTEX_H

#include "onix/types.h"
#include "onix/list.h"

typedef struct mutex_t
{
    bool value;
    list_t waiters;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutext);
void mutex_unlock(mutex_t *mutex);

typedef struct spinlock_t
{
    struct task_t *holder; // 持有者
    mutex_t mutex;         // 互斥量
    u32 repeat;            // 重入次数
} spinlock_t;

void spin_init(spinlock_t *lock);   // 锁初始化
void spin_lock(spinlock_t *lock);   // 加锁
void spin_unlock(spinlock_t *lock); // 解锁

#endif
