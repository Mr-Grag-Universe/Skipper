section .text
    global my_export_asm_factorial

my_export_asm_factorial:
    ; Вход: rdi - число n
    ; Выход: rax - факториал n

    ; Проверка на n <= 1
    cmp rdi, 1
    jle .end

    ; Инициализация результата
    mov rax, 1

.loop:
    ; Умножаем результат на текущее значение n
    mul rdi
    ; Уменьшаем n
    dec rdi
    ; Проверяем, не достигли ли мы 1
    cmp rdi, 1
    jg .loop

.end:
    ret
