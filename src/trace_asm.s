0000000072015460 <_Z14trace_overflowPijjj>:

-> 1. rdi = int* offset
-> 2. esi = size
-> 3. edx = uint32_t ind
-> 4. ecx = reg_id

add    $0x1,%edx    // ind+1
// сохраняем r15
push   %r15

// сохраняем r14 и переносим в него аргумент
// ecx -> r14 -> rdi для get_msb_ind
push   %r14
mov    %ecx,%r14d

push   %r13

// сохраняем r12 и переносим в него offset
push   %r12
mov    %rdi,%r12

// сохраняем rbp
push   %rbp
mov    %esi,%ebp    // rsi = ebp = size

// сохраняем ebx
// ebx = 65*(ind+1)
push   %rbx
mov    %edx,%ebx    // edx = ebx (=(ind+1))
shl    $0x6,%ebx    // edx *= 2^6 (= edx*64)
add    %edx,%ebx    // edx += ebx

// выделение места (видимо под dr_mcontext_t)
sub    $0x918,%rsp

// if (...) print
cmp    %ebx,%esi    // if (ebx < esi)
jb     72015510 <_Z14trace_overflowPijjj+0xb0>
xor    %eax,%eax    // eax = 0

mov    %rsp,%rdi
mov    $0x122,%ecx  // ecx = 290
mov    %rsp,%r15    // r15 = rsp
rep stos %rax,%es:(%rdi)
// запись полей в стуктуру 
// dr_mcontext_t mc = { sizeof(mc), DR_MC_ALL}
movq   $0x910,(%rsp)    // *(rsp+...) = 2320
movl   $0x7,0x8(%rsp)   // *(rsp+...) = 7

// dr_get_mcontext(dr_get_current_drcontext(), &mc);
call   7200eba0 <dr_get_current_drcontext@plt>
mov    %r15,%rsi    // 2 аргумент
mov    %rax,%rdi    // 1 аргумент
call   7200ea90 <dr_get_mcontext@plt>
movzwl %r14w,%edi
mov    %r15,%rsi

// reg_t xflags = mc.xflags
mov    0x90(%rsp),%r13

// reg_t reg = reg_get_value(dst_reg, &mc);
call   7200eee0 <reg_get_value@plt>
mov    %eax,%edi    // перекладываем результат reg_get_value в первый аргумент get_msb_ind

// msb_ind_reg = get_msb_ind((uint) reg)
call   7200eae0 <_Z11get_msb_indj@plt>

// if (msb_ind_reg >= 0) {
//        ((char *)offset)[(ind*65+msb_ind_reg) % size] += 1;
// }
test   %eax,%eax            // msb_ind_reg >= 0
js     720154e7 <_Z14trace_overflowPijjj+0x87>
lea    -0x41(%rax,%rbx,1),%eax
xor    %edx,%edx
div    %ebp                 // % size
addb   $0x1,(%r12,%rdx,1)

lea    -0x1(%rbx),%eax
xor    %edx,%edx            // edx = 0
and    $0x1,%r13d           // xflags & EFLAGS_CF (EFLAGS_CF = 1)
div    %ebp                 // % size
xor    %eax,%eax            // eax = 0 (видимо код возврата)
add    %r13b,(%r12,%rdx,1)

// удаляем локальные переменные
add    $0x918,%rsp

// восстанавливаем регистры
pop    %rbx
pop    %rbp
pop    %r12
pop    %r13
pop    %r14
pop    %r15
ret

// видимо это код принта
nopw   0x0(%rax,%rax,1)
lea    0x1bce1(%rip),%rdi        # 720311f8 <_fini+0x1c9c>
call   7200ee30 <puts@plt>
jmp    7201548b <_Z14trace_overflowPijjj+0x2b>
data16 cs nopw 0x0(%rax,%rax,1)

nopl   0x0(%rax)