[bits 32]

extern kernel_init
extern memory_init
extern console_init

global _start
_start:
    ;call kernel_init
    push ebx ;ards_count
    push eax;magic
    call console_init;
    call memory_init ;
    
    jmp $