trace_overflow:

// rdi = (int*)     offset
// esi = (uint32_t) size
// edx = (uint32_t) ind
// ecx = (uint32_t) reg_id

push r15
push r14
push r13
push r12
push rbp
push rbx

// ebx = 65*(ind+1)
add     $0x1,%edx
mov     %edx,%ebx
shl     $0x6,%ebx
add     %edx,%ebx

// if (ebx < size) - недостаточно памяти
cmp     %ebx,%esi
jb      xxx // надо прыгнуть чуть вперёд
xor     %eax,%eax
ret

// надо получить флаги
// пусть положим их в r13



start:
    test    %rcx,%rcx
    je      ret_minus_1
    xor     %rax,%rax

    shr     %rcx
    je      return
loop:
    inc     %rax
    shr     %rcx
    jne     loop
    jmp     return

ret_minus_1:
    mov -1,%rax
return:
    ret