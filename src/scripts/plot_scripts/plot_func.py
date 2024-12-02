import matplotlib.pyplot as plt
import numpy as np
import argparse
from sklearn.linear_model import LinearRegression
import os

def read_numbers_from_file(filename):
    """Считывает числа из файла, каждое число с новой строки."""
    with open(filename, 'r') as file:
        numbers = [float(line.strip()) for line in file if line.strip()]
    return numbers

def remove_outliers(x, y, threshold=5):
    """Удаляет выбросы, которые находятся на расстоянии больше threshold стандартных отклонений от линии регрессии."""
    x = np.array(x).reshape(-1, 1)
    y = np.array(y)
    
    # Линейная регрессия
    reg = LinearRegression().fit(x, y)
    y_pred = reg.predict(x)
    
    # Вычисление отклонений и стандартного отклонения
    residuals = y - y_pred
    std_dev = np.std(residuals)
    print(threshold * std_dev)
    
    # Определение индексов, которые не являются выбросами
    inliers = np.abs(residuals) <= threshold * std_dev
    
    return x[inliers], y[inliers]

def plot_numbers(numbers, n_std=5, save_path="combined_plot.png", show=1):
    """Строит два графика: линейный и scatter plot с устранением выбросов."""
    # Индексы времени
    time = np.arange(len(numbers))
    
    # Удаление выбросов
    time_inliers, numbers_inliers = remove_outliers(time, numbers, n_std)
    
    # Создание фигуры с двумя подграфиками
    fig, axs = plt.subplots(2, 1, figsize=(10, 12))
    
    # Линейный график
    axs[0].plot(time, numbers, marker='o', linestyle='-', color='b')
    axs[0].set_title('Время исполнения vs итерация')
    axs[0].set_ylabel('Время, с')
    axs[0].set_xlabel('Итерации, №')
    axs[0].grid(True)
    
    # Scatter plot с устранением выбросов
    axs[1].scatter(time_inliers, numbers_inliers, color='r')
    axs[1].set_title('Время исполнения vs итерация (сlipped)')
    axs[1].set_ylabel('Время, с')
    axs[1].set_xlabel('Итерация, №')
    axs[1].grid(True)
    
    # Сохранение графиков в файл
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    plt.savefig(save_path)
    
    # Показ графиков
    if show:
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Построение графиков зависимости чисел от времени.')
    parser.add_argument('--data_path', type=str, help='Имя файла с числами.')
    parser.add_argument('--n_std', type=float, help='Число отклонений для того чтобы посчитать выбросом.')
    parser.add_argument('--save_path', type=str, help='Путь для сохранения графика')
    parser.add_argument('--show', type=int, help='0 / 1 - Делаем show или нет')
    args = parser.parse_args()
    print(args)

    numbers = read_numbers_from_file(args.data_path)
    plot_numbers(numbers, args.n_std, args.save_path, args.show)

if __name__ == '__main__':
    main()
