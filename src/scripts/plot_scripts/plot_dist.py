import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import argparse
import os


parser = argparse.ArgumentParser(description='Построение графиков зависимости чисел от времени.')
parser.add_argument('--data_path', type=str, help='Имя файла с числами.')
parser.add_argument('--save_path', type=str, help='Путь для сохранения графика')
parser.add_argument('--show', type=int, help='0 / 1 - Делаем show или нет')
args = parser.parse_args()
print(args)

# Чтение данных из файла
execs = []
ex_lens = []
iterations = []
with open(args.data_path) as f:
    for line in f.readlines():
        vals = np.array(list(map(int, line.split(',')[:-1])))
        if len(vals):
            ex_lens.append(len(vals))
            execs.extend(vals)
            iterations.extend([len(ex_lens)] * len(vals))

execs = np.array(execs)
iterations = np.array(iterations)

print(execs)
print(ex_lens)

# Создание DataFrame
data = {
    'iteration': iterations.astype(int),
    'f': execs
}

df = pd.DataFrame(data)
print(df)

# Определение границ по оси x
x_min = df['iteration'].min()
x_max = df['iteration'].max()

# Создание фигуры с двумя подграфиками
fig, axs = plt.subplots(2, 1, figsize=(12, 16))

# Гистограмма
sns.histplot(data=df, x='iteration', y='f', bins=30, pthresh=0.1, cmap='viridis', ax=axs[0])
axs[0].set_title('Distribution of [execs/s] Across Iterations')
axs[0].set_xlabel('Iteration')
axs[0].set_ylabel('Parameter [execs/s]')
axs[0].set_xlim(x_min, x_max)

print("one is ready")
print(df.groupby('iteration').agg('mean'))

# График линий уровня с настройками для улучшения отображения
sns.kdeplot(data=df.astype(float), x='iteration', y='f', cmap='viridis', fill=True, thresh=0, levels=100, ax=axs[1], alpha=0.5)
axs[1].set_title('KDE Plot of [execs/s] Across Iterations')
axs[1].set_xlabel('Iteration')
axs[1].set_ylabel('Parameter [execs/s]')
axs[1].set_xlim(x_min, x_max)

print("two are ready")

if x_max <= 10:
    axs[0].set_xticks(range(1, x_max+1))
    axs[1].set_xticks(range(1, x_max+1))

print("making dirs and saving image")
# Сохранение графиков в файл
os.makedirs(os.path.dirname(args.save_path), exist_ok=True)
plt.savefig(args.save_path)

# Показ графиков
if args.show:
    plt.show()
