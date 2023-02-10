[bits 32]

magic   equ 0xe85250d6
i386    equ 0
length  equ header_end - header_start

section .multiboot2
header_start:
    dd magic  ; 魔数
    dd i386   ; 32位保护模式
    dd length ; 头部长度
    dd -(magic + i386 + length); 校验和

    ; 结束标记
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

extern device_init
extern kernel_init
extern memory_init
extern console_init
extern gdt_init
extern gdt_ptr

code_selector equ (1 << 3)
data_selector equ (2 << 3)

section .text
global _start
_start:
    ;call kernel_init
    push ebx ;ards_count
    push eax;magic

    call device_init    ; 虚拟设备初始化
    call console_init;
    
    call gdt_init
    
    lgdt [gdt_ptr]

    jmp dword code_selector:_next
_next:

    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax; 初始化段寄存器

    call memory_init ;

    mov esp, 0x10000; 修改栈顶
    
    call kernel_init
    
    jmp $