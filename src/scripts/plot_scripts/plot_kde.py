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
    
    # Строим kde с помощью seaborn
    sns.kdeplot(combined_execs, fill=True, alpha=0.5, ax=ax)
    
    # Устанавливаем заголовок и метки осей
    ax.set_title('KDE [execs/s]')
    ax.set_xlabel('[execs/s]')
    ax.set_ylabel('Плотность')

def plot_times(times, ax):
    # Строим kde с помощью seaborn
    sns.kdeplot(times, fill=True, alpha=0.5, ax=ax)
    
    # Устанавливаем заголовок и метки осей
    ax.set_title('KDE времени исполнения')
    ax.set_xlabel('Время, с')
    ax.set_ylabel('Плотность')

def main():
    parser = argparse.ArgumentParser(description='Построение kde.')
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
