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
        # удалим crash-* файл
        subprocess.run("rm crash-*", cwd=working_directory, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        delta = end_time - start_time
        total_time += delta
        time_distribution.append(delta)

        execs = []
        for line in result.stderr.splitlines():
            match = exec_s_pattern.search(line)
            if match:
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

    with open("out/timing/time.txt", "w") as f:
        f.write('\n'.join(list(map(str, time_distribution))))
    with open("out/timing/exec.txt", "w") as f:
        for l in exec_lists:
            f.write(','.join(list(map(str, l))) + "\n")

if __name__ == "__main__":
    main()
