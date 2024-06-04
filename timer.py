import sys
import subprocess
import time
import os
import argparse
import matplotlib.pyplot as plt
import seaborn as sns
import re

def main():
    parser = argparse.ArgumentParser(description="Measure the time taken to execute a command N times.")
    parser.add_argument('--n_iter', nargs=1, type=int, help="Number of times to execute the command")
    parser.add_argument('--command', nargs=1, type=str, help="The command to execute")

    args = parser.parse_args()

    n_iter = args.n_iter[0]
    working_directory = os.getcwd()
    command = args.command[0]
    print(f"<{n_iter}>, <{working_directory}>, <{command}>")

    if not command:
        print("No command provided to execute.")
        sys.exit(1)

    total_time = 0.0
    exec_s_pattern = re.compile(r'exec/s:\s(\d+)')

    time_distribution = []
    exec_s_distribution = []
    exec_lists = []
    for i in range(1, n_iter + 1):
        start_time = time.time()
        result = subprocess.run(command, cwd=working_directory, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        end_time = time.time()
        delta = end_time - start_time
        total_time += delta
        time_distribution.append(delta)

        execs = []
        for line in result.stderr.splitlines():
            # print(line)
            match = exec_s_pattern.search(line)
            # print(match)
            if match:
                # execs.append(int(match.group(1)))
                # print("match: ", match.group(0))
                ex = int(match.group(1))
                if ex != 0:
                    execs.append(ex)
                    exec_s_distribution.append(ex)
        exec_lists.append(execs)

        if i % 5 == 0:
            print(f"Completed {i} iterations")

    average_time = total_time / n_iter

    print(f"Total time for {n_iter} executions: {total_time:.6f} seconds")
    print(f"Average time per execution: {average_time:.6f} seconds")

    # print(time_distribution)
    # print(exec_s_distribution)

    fig, axs = plt.subplots(ncols=2, nrows=1, figsize=(15, 8))

    ax1 = sns.kdeplot(time_distribution, fill=True, alpha=0.5, ax=axs[0])
    sns.rugplot(time_distribution, color='black', ax=ax1)
    axs[0].set_title('Execution Time')
    axs[0].set_xlabel('time [s]')
    axs[0].set_ylabel('KDE')

    ax2 = sns.kdeplot(exec_s_distribution, fill=True, alpha=0.5, ax=axs[1])
    # sns.rugplot(exec_s_distribution, color='black', ax=ax2)
    axs[1].set_title('Executions per second')
    axs[1].set_xlabel('execotions per second [exec/s]')
    axs[1].set_ylabel('KDE')

    fig.savefig('out/timing/density_plot.png')

    with open("out/timing/time.txt", "w") as f:
        f.write('\n'.join(list(map(str, time_distribution))))
    with open("out/timing/exec.txt", "w") as f:
        for l in exec_lists:
            f.write(','.join(list(map(str, l))) + "\n")

    # plt.show()

if __name__ == "__main__":
    main()
