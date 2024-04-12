bits    64

section .data
    format db 'Hello, world!', 10, 0  ; форматная строка для printf (10 - символ новой строки, 0 - нуль-терминатор)
    msg db "register: %d!!!", 10, 0    ; Форматная строка для вывода числа с переводом строки

section .text
global _asm_sort
extern printf                 ; объявляем внешнюю функцию printf

extern	printf

print_number:
    ; Пролог функции
    push rbp              ; Сохраняем значение базового указателя стека
    mov rbp, rsp          ; Устанавливаем базовый указатель стека

    ; Основная часть функции
    push rdi
    mov rdi, msg
    pop rsi
    xor	rax, rax
    call printf
    mov rdi, rsi

    ; Эпилог функции
    pop rbp               ; Восстанавливаем базовый указатель стека
    ret                   ; Возвращаемся из функции

_asm_sort:
    push rbp
    mov rbp, rsp

    ; push rsi
    ; push rdi

    mov rdx, rdi   ; загружаем указатель на массив в регистр edx
    mov rcx, rsi   ; загружаем длину массива в регистр ecx
    add rcx, rdi

    ; pop rdi
    ; call print_number
    ; pop rdi
    ; call print_number

    ; leave
    ; ret

    xor rsi, rsi           ; начинаем с индекса i = 0

outer_loop:
    cmp rdx, rcx           ; проверяем, достигли ли конца массива
    jge end_outer_loop     ; если да, завершаем внешний цикл

    mov rdi, rdx           ; начинаем с индекса j = i

inner_loop:
    cmp rdi, rcx           ; проверяем, достигли ли конца массива
    jge end_inner_loop     ; если да, завершаем внутренний цикл

    mov al, byte [rdx]  ; загружаем a[i] в регистр eax
    mov ah, byte [rdi]  ; загружаем a[j] в регистр ebx
    cmp al, ah                ; сравниваем a[i] и a[j]

    jle no_swap               ; если a[i] <= a[j], пропускаем обмен

    ; если a[i] > a[j], производим обмен
    mov [rdx], ah  ; a[i] = a[j]
    mov [rdi], al  ; a[j] = a[i]

    ; ; информирование
    ; push rdi
    ; mov rdi, format               ; передаем адрес форматной строки в rdi
    ; call printf                   ; вызываем функцию printf
    ; pop rdi

no_swap:
    inc rdi                ; инкрементируем индекс j
    jmp inner_loop         ; переходим в начало внутреннего цикла

end_inner_loop:
    inc rdx                ; инкрементируем индекс i
    jmp outer_loop         ; переходим в начало внешнего цикла

end_outer_loop:
    leave
    ret
