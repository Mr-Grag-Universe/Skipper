#!/bin/zsh

# Проверка наличия хотя бы двух аргументов
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 origin/instr n1 [n2 ... nn]"
    exit 1
fi

# Первый аргумент - имя скрипта
type=$1

# Удаляем первый аргумент, чтобы остались только значения n
shift

# Итерация по всем переданным значениям n
for n in "$@"
do
    # Запуск команды с подстановкой параметра n
    python plot_scripts/plot_dist.py --data_path=./out/timing/"$n"/"$type"/data/exec.txt --save_path=./out/timing/"$n"/"$type"/img/dist.png --show=0
    python plot_scripts/plot_func.py --n_std=0.5 --data_path=./out/timing/"$n"/"$type"/data/time.txt --save_path=./out/timing/"$n"/"$type"/img/func.png --show=0
    python plot_scripts/plot_kde.py --data_path=./out/timing/"$n"/"$type"/data/exec.txt,./out/timing/"$n"/"$type"/data/time.txt --save_path=./out/timing/"$n"/"$type"/img/kde2.png --show=0
    python plot_scripts/plot_hist.py --data_path=./out/timing/"$n"/"$type"/data/exec.txt,./out/timing/"$n"/"$type"/data/time.txt --save_path=./out/timing/"$n"/"$type"/img/hist.png --show=0

    # Здесь можно добавить дополнительные команды, аналогичные приведенной выше
    # python plot_scripts/plot_another.py --data_path=./out/timing/"$n"/instr/data/another.txt --save_path=./out/timing/"$n"/instr/img/another.png
    # python plot_scripts/plot_yet_another.py --data_path=./out/timing/"$n"/instr/data/yet_another.txt --save_path=./out/timing/"$n"/instr/img/yet_another.png

done

echo "All tasks completed."
