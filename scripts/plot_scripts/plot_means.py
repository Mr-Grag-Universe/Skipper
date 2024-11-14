import json
import argparse
import matplotlib.pyplot as plt


def plot_comp(arr_1, arr_2, log_y=False, x='x', y='y', title="", save=False, show=True, save_path=None):
    assert len(arr_1) == len(arr_2), "length of arrays must be the same"
    n = len(arr_1)

    fig, axs = plt.subplot_mosaic("AA\nBB\nCC", figsize=(10, 16), sharex=True)

    axs['A'].plot(range(1, n+1), arr_1, marker='h')
    axs['C'].plot(range(1, n+1), arr_2, marker='h')
    axs['B'].plot(range(1, n+1), arr_1, marker='h')
    axs['B'].plot(range(1, n+1), arr_2, marker='h')

    fig.suptitle(f"Mean {title} Plot", fontsize=16)
    axs['A'].set_title("Original Fuzzer", loc='right')
    axs['B'].set_title("Comparison", loc='right')
    axs['C'].set_title("Custom Fuzzer", loc='right')

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

    if save:
        assert save_path is not None, "save path is None! cannot save this picture!"
        fig.savefig(save_path)

    if show:
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Calculate mean values from experiment files and save to JSON.')
    parser.add_argument('--input', type=str, required=True, help='JSON file with statistics')
    parser.add_argument('--output', type=str, required=True, help='Output graphic for statistics in input files')
    args = parser.parse_args()

    m = None
    with open(args.input, "r") as input_file:
        m = json.load(input_file)
    
    plot_comp(m['origin']['time'], m['instr']['time'], log_y=(0, 0, 0), x='execution, №', y='time, s', title="Time")
    plot_comp(m['origin']['execs/s'], m['instr']['execs/s'], log_y=(0, 1, 0), x='execution, №', y='execs/s', title="Execution Speed")

