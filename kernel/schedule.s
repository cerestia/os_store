global task_switch
task_switch:
    push ebp
    mov ebp,esp

    push ebx
    push esi
    push edi

    mov eax,esp
    and eax,0xfffff000
    ;将esp放入页起始，esp切换为next的esp
    mov [eax],esp
    mov eax,[ebp+8]
    mov esp,[eax]
    ;已经切换到新的任务栈
    pop edi
    pop esi
    pop ebx
    pop ebp

    ret