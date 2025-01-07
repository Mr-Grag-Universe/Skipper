#!/bin/zsh

# приничает 1 аргумент:
# auto - автоматически добавит 3 пути в path
# <path> - запишет что-то ещё в path
# ничего - ничего не добавит в path
#
# в любом случае обновит значения PATH_TO_LIBFUZZER переменных

# Массив путей для автоматического добавления
auto_paths=(
    "/home/debian/Skipper/LLVM/llvm-project/build"
    "/home/debian/Skipper/LLVM/llvm-project/build/lib"
    "/home/debian/Skipper/LLVM/llvm-project/build/bin"
)

# Проверяем, был ли передан аргумент
if [ -n "$1" ]; then
    if [ "$1" = "auto" ]; then
        # Итерируемся по массиву и добавляем каждый путь в PATH
        for p in "${auto_paths[@]}"; do
            # echo "PATH: ${PATH}"
            export PATH="${p}:${PATH}"
            echo "Автоматически добавлен путь '${p}' в PATH."
        done
    else
        edit_path="$1"

        # Проверяем, существует ли указанный путь
        if [ ! -d "$edit_path" ]; then
            echo "Ошибка: Указанный путь '$edit_path' не существует."
            exit 1
        fi

        # Добавляем путь в переменную PATH
        export PATH="${edit_path}:${PATH}"
        echo "Путь '${edit_path}' был добавлен в PATH."
    fi
else
    echo "Аргумент не передан. Обновление PATH пропущено."
fi

export PATH_TO_LIBFUZZER=/home/debian/Skipper/LLVM/llvm-project/build/lib/clang/20/lib/x86_64-unknown-linux-gnu/libclang_rt.fuzzer.a
# export PATH_TO_LIBFUZZER=/usr/lib/llvm-14/lib/clang/14.0.6/lib/linux/libclang_rt.fuzzer-x86_64.a

export PATH_TO_LIBFUZZER_DIR=/home/debian/Skipper/LLVM/llvm-project/build/lib/clang/20/lib/x86_64-unknown-linux-gnu/
# export PATH_TO_LIBFUZZER_DIR=/home/debian/Skipper/LLVM/llvm-project/build/projects/compiler-rt/lib/fuzzer
# export PATH_TO_LIBFUZZER_DIR=/home/debian/Skipper/LLVM/llvm-project/compiler-rt/lib/fuzzer
# export PATH_TO_LIBFUZZER_DIR=/usr/lib/llvm-14/lib/clang/14.0.6/lib/linux

# Выводим установленные переменные
echo "PATH_TO_LIBFUZZER: $PATH_TO_LIBFUZZER"
echo "PATH_TO_LIBFUZZER_DIR: $PATH_TO_LIBFUZZER_DIR"