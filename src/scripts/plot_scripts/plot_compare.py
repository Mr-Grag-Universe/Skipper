import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import argparse
import os
import scipy as sc


def read_time(path):
    """Считывает числа из файла, каждое число с новой строки."""
    with open(path, 'r') as file:
        numbers = [float(line.strip()) for line in file if line.strip()]
    return numbers

def read_exec(path):
    execs = []
    with open(path) as f:
        for line in f.readlines():
            vals = list(map(int, line.split(',')[:-1]))
            if len(vals):
                execs.append(vals)
    return execs

def plot_execs(execs, ax, label=None):
    combined_execs = np.concatenate(execs)
    print("execs: ", np.mean(combined_execs), np.std(combined_execs, ddof=1))
    
    # Строим kde с помощью seaborn
    # sns.kdeplot(combined_execs, fill=True, alpha=0.5, ax=ax, label=label)
    sns.histplot(combined_execs, kde=True, ax=ax, binwidth=200, alpha=0.5, label=label, stat='density')

    # Устанавливаем заголовок и метки осей
    ax.set_title('Гистограмма скорости фаззига [execs/s]')
    ax.set_xlabel('[execs/s]')
    ax.set_ylabel('Вероятность')

def plot_times(times, ax, label=None):
    # Строим kde с помощью seaborn
    # sns.kdeplot(times, fill=True, alpha=0.5, ax=ax, label=label)
    sns.histplot(times, kde=True, ax=ax, binwidth=10, alpha=0.5, label=label, stat='density')
    
    # Устанавливаем заголовок и метки осей
    ax.set_title('Гисторгамма времени фаззинга')
    ax.set_xlabel('Время, с')
    ax.set_ylabel('Вероятность')
    ax.set_xlim(0, np.max(times)*1.01)
    ax.set_xticks(np.arange(0, int(np.max(times)), 10))

def main():
    parser = argparse.ArgumentParser(description='Построение сравнительных графиков.')
    parser.add_argument('--show', type=int, help='0 / 1 - Делаем show или нет')
    args = parser.parse_args()

    base_path = '/home/debian/Skipper/out/timing'
    path_exec, path_time = base_path+'/with_asm/exec.txt', base_path+'/with_asm/time.txt'
    execs_1 = read_exec(path_exec)
    times_1 = read_time(path_time)
    print(np.mean(times_1), np.std(times_1, ddof=1))
    path_exec, path_time = base_path+'/without_asm/exec.txt', base_path+'/without_asm/time.txt'
    execs_2 = read_exec(path_exec)
    times_2 = read_time(path_time)
    print(np.mean(times_2), np.std(times_2, ddof=1))

    fig, axs = plt.subplots(1, 1, figsize=(12, 8))
    plot_execs(execs_1, axs, "execs with_asm")
    plot_execs(execs_2, axs, "execs without_asm")
    axs.legend()
    save_path = '/home/debian/Skipper/out/imgs/compare_speed.png'
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    plt.savefig(save_path)

    if args.show:
        plt.show()

    fig, axs = plt.subplots(1, 1, figsize=(12, 8))
    plot_times(times_1, axs, "time with_asm")
    plot_times(times_2, axs, "time without_asm")
    axs.legend()
    save_path = '/home/debian/Skipper/out/imgs/compare_time.png'
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    plt.savefig(save_path)

    if args.show:
        plt.show()


if __name__ == '__main__':
    main()
