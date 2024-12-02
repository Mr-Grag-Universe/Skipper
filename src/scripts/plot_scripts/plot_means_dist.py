from operator import floordiv
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import argparse
import json
import os
from collections import Counter
from scipy import stats


def calculate_mean_from_file(filepath):
    means = []
    with open(filepath, 'r') as file:
        for line in file:
            line = line.strip('\n')
            line = line.strip(' ')
            if len(line):
                numbers = list(map(float, line.split(',')))
                if numbers:
                    means.append(np.mean(numbers))
    return means

def find_kde_max(data):
    # Строим KDE
    kde = stats.gaussian_kde(data)

    # Создаем линейное пространство для оценки KDE
    x = np.linspace(min(data), max(data), 1000)

    # Оцениваем KDE на этом пространстве
    kde_values = kde(x)

    # Находим точку максимума
    max_index = np.argmax(kde_values)
    max_value = x[max_index]
    
    return max_value

def plot_comp(arr_1, arr_2, log_y=False, x='x', y='y', title="", show=True, save_path=None, tr=None):
    assert len(arr_1) == len(arr_2), "length of arrays must be the same"
    n = len(arr_1)

    # arr_1 = np.array(arr_1)
    # arr_2 = np.array(arr_2)

    means_1 = np.array(list(map(lambda x: np.mean(x), arr_1))).flatten()
    means_2 = np.array(list(map(lambda x: np.mean(x), arr_2))).flatten()

    most_common_1 = np.array(list(map(find_kde_max, arr_1))).flatten()
    most_common_2 = np.array(list(map(find_kde_max, arr_2))).flatten()

    print(most_common_1)

    df_1 = {"iteration" : [], "f" : []}
    for i, line in enumerate(arr_1):
        df_1["iteration"] += [i+1] * len(line)
        df_1["f"] += list(line)

    df_2 = {"iteration" : [], "f" : []}
    for i, line in enumerate(arr_2):
        df_2["iteration"] += [i+1] * len(line)
        df_2["f"] += list(line)

    df_1 = pd.DataFrame(df_1)
    df_2 = pd.DataFrame(df_2)

    fig, axs = plt.subplot_mosaic("AA\nBB\nCC", figsize=(10, 16), sharex=True)

    axs['A'].plot(range(1, n+1), means_1, marker='h', c='red', label='mean')
    axs['C'].plot(range(1, n+1), means_2, marker='h', c='red', label='mean')

    axs['A'].plot(range(1, n+1), most_common_1, marker='h', c='orange', label='kde max')
    axs['C'].plot(range(1, n+1), most_common_2, marker='h', c='orange', label='kde max')

    axs['B'].plot(range(1, n+1), means_1, marker='h', label='original')
    axs['B'].plot(range(1, n+1), means_2, marker='h', label='custom')

    if tr is None:
        tr_1 = np.max(df_1)
        tr_2 = np.max(df_2)
        tr = max(tr_1, tr_2)
    
    sns.histplot(data=df_1[df_1 < tr], x='iteration', y='f', pthresh=0.1, cmap='viridis', ax=axs['A'], discrete=True)
    sns.histplot(data=df_2[df_2 < tr], x='iteration', y='f', pthresh=0.1, cmap='viridis', ax=axs['C'], discrete=True)

    print(df_1)
    
    fig.suptitle(f"Mean {title} Plot", fontsize=16)
    axs['A'].set_title("Original Fuzzer", loc='right')
    axs['B'].set_title("Comparison", loc='right')
    axs['C'].set_title("Custom Fuzzer", loc='right')
    axs['A'].legend()
    axs['B'].legend()
    axs['C'].legend()

    axs['C'].set_xlabel(x)
    axs['A'].set_ylabel(y)
    axs['B'].set_ylabel(y)
    axs['C'].set_ylabel(y)

    if any(log_y):
        if log_y[0]:
            axs['A'].set_yscale('log')
        if log_y[1]:
            axs['B'].set_yscale('log')
        if log_y[2]:
            axs['C'].set_yscale('log')

    axs['C'].set_xlim(1, len(arr_1))

    for ax in axs.values():
        ax.grid(True, axis='x')

    # fig.tight_layout()

    if save_path:
        assert save_path is not None, "save path is None! cannot save this picture!"
        fig.savefig(save_path)

    if show:
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Calculate mean values from experiment files and save to JSON.')
    parser.add_argument('--timing_dir', type=str, required=True, help='Directory containing the experiment results.')
    parser.add_argument('--output', type=str, required=True, help='Output JSON file.')
    args = parser.parse_args()
    
    # собираем данные
    data =  {
                "exec" : {"origin" : [], "instr" : []},
                "time" : {"origin" : [], "instr" : []}
            }
    n_dirs = sorted(os.listdir(args.timing_dir))
    for i in n_dirs:
        n_path = os.path.join(args.timing_dir, str(i))
        for t in ["origin", "instr"]:
            for val in ["exec", "time"]:
                path = os.path.join(n_path, f"{t}/data/{val}.txt")
                if os.path.exists(path):
                    means = calculate_mean_from_file(path)
                    means = list(map(lambda x: [np.mean(x)], means))
                    print(f"<{t}>, <{val}> : ", len(means))
                    dist = np.asarray(means).flatten()
                    data[val][t].append(dist)
    # print(data)

    # строим по данным 
    plot_comp(
                data['time']['origin'], 
                data['time']['instr'], 
                log_y=(0, 0, 0), 
                x='execution, №', 
                y='time, s', 
                title="Time", 
                tr=125, 
                save_path=os.path.join(args.output, "mean_time_dist.png"))
    plot_comp(
                data['exec']['origin'], 
                data['exec']['instr'], 
                log_y=(0, 1, 0), 
                x='execution, №', 
                y='execs/s', 
                title="Execution Speed", 
                tr=None, 
                save_path=os.path.join(args.output, "mean_exec_dist.png"))


