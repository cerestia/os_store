#include "onix/mutex.h"
#include "onix/task.h"
#include "onix/interrupt.h"
#include "onix/assert.h"

void mutex_init(mutex_t *mutex)
{
    mutex->value = false;
    list_init(&mutex->waiters);
}

// 尝试持有信号量
void mutex_lock(mutex_t *mutex)
{
    bool intr = interrupt_disable();

    task_t *current = running_task();
    while (mutex->value == true)
    {
        // 加入锁的等待队列
        task_block(current, &mutex->waiters, TASK_BLOCKED);
    }
    // 判断无人持有
    assert(mutex->value == false);
    // 持有
    mutex->value++;
    assert(mutex->value == true);

    // 恢复之前的中断状态
    set_interrupt_state(intr);
}

void mutex_unlock(mutex_t *mutex)
{
    // 关闭中断，保证原子操作
    bool intr = interrupt_disable();

    // 已持有互斥量
    assert(mutex->value == true);

    // 取消持有
    mutex->value--;
    assert(mutex->value == false);

    // 如果等待队列不为空，则恢复执行
    if (!list_empty(&mutex->waiters))
    {
        task_t *task = element_entry(task_t, node, mutex->waiters.tail.prev);
        assert(task->magic == ONIX_MAGIC);
        task_unblock(task);
        // 保证新进程能获得互斥量，不然可能饿死
        task_yield();
    }

    // 恢复之前的中断状态
    set_interrupt_state(intr);
}

void lock_init(lock_t *lock)
{
    lock->holder = NULL;
    lock->repeat = 0;
    mutex_init(&lock->mutex);
}

void lock_acquire(lock_t *lock)
{
    task_t *current = running_task();
    if (lock->holder != current)
    {
        mutex_lock(&lock->mutex);
        lock->holder = current;
        assert(lock->repeat == 0);
        lock->repeat = 1;
    }
    else
    {
        lock->repeat++;
    }
}

void lock_release(lock_t *lock)
{
    task_t *current = running_task();
    assert(lock->holder == current);
    if (lock->repeat > 1)
    {
        lock->repeat--;
        return;
    }

    assert(lock->repeat == 1);

    lock->holder = NULL;
    lock->repeat = 0;
    mutex_unlock(&lock->mutex);
}