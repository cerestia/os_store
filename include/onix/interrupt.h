#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include <onix/types.h>

#define IDT_SIZE 256

#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘
#define IRQ_CASCADE 2    // 8259 从片控制器
#define IRQ_SERIAL_2 3   // 串口 2
#define IRQ_SERIAL_1 4   // 串口 1
#define IRQ_PARALLEL_2 5 // 并口 2
#define IRQ_FLOPPY 6     // 软盘控制器
#define IRQ_PARALLEL_1 7 // 并口 1
#define IRQ_RTC 8        // 实时时钟
#define IRQ_REDIRECT 9   // 重定向 IRQ2
#define IRQ_MOUSE 12     // 鼠标
#define IRQ_MATH 13      // 协处理器 x87
#define IRQ_HARDDISK 14  // ATA 硬盘第一通道
#define IRQ_HARDDISK2 15 // ATA 硬盘第二通道

#define IRQ_MASTER_NR 0x20 // 主片起始向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始向量号

typedef struct gate_t
{
    u16 offset0;
    u16 selector;
    u8 reserved;
    u8 type : 4;
    u8 segment : 1;
    u8 DPL : 2;
    u8 present : 1;
    u16 offset1;
} _packed gate_t;

typedef void *handler_t; //中断处理函数

void send_eoi(int vector);

void set_interrupt_handler(u32 irq, handler_t handler);
void set_interrupt_mask(u32 irq, bool enable);

bool interrupt_disable();             // 清除 IF 位，返回设置之前的值
bool get_interrupt_state();           // 获得 IF 位
void set_interrupt_state(bool state); // 设置 IF 位

#endif