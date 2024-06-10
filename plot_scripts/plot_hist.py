import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse
import os

def read_time(filename):
    """Считывает числа из файла, каждое число с новой строки."""
    with open(filename, 'r') as file:
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

def plot_execs(execs, ax):
    combined_execs = np.concatenate(execs)
    
    # Строим гистограмму с помощью seaborn
    sns.histplot(combined_execs, kde=True, ax=ax, binwidth=1000)
    
    # Устанавливаем заголовок и метки осей
    ax.set_title('Гистограмма [execs/s]')
    ax.set_xlabel('[execs/s]')
    ax.set_ylabel('Частота')

def plot_times(times, ax):
    # Строим гистограмму с помощью seaborn
    sns.histplot(times, kde=True, ax=ax, binwidth=5)
    
    # Устанавливаем заголовок и метки осей
    ax.set_title('Гистограмма времени исполнения')
    ax.set_xlabel('Время, с')
    ax.set_ylabel('Частота')

def main():
    parser = argparse.ArgumentParser(description='Построение гистограмм.')
    parser.add_argument('--data_path', type=str, help='Пути к файлам с exec и time.')
    parser.add_argument('--save_path', type=str, help='Путь для сохранения графика')
    parser.add_argument('--show', type=int, help='0 / 1 - Делаем show или нет')
    args = parser.parse_args()
    print(args)

    path_exec, path_time = args.data_path.split(',')

    execs = read_exec(path_exec)
    times = read_time(path_time)
    print(execs, times)

    fig, axs = plt.subplots(2, 1, figsize=(12, 16))
    plot_execs(execs, axs[0])
    plot_times(times, axs[1])

    os.makedirs(os.path.dirname(args.save_path), exist_ok=True)
    plt.savefig(args.save_path)

    if args.show:
        plt.show()


if __name__ == '__main__':
    main()
