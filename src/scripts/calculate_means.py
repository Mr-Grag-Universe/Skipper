import os
import json
import numpy as np
import argparse

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

def main(timing_dir, output_file):
    results = {'origin': {'execs/s': [], 'time': []}, 'instr': {'execs/s': [], 'time': []}}
    
    n_dirs = sorted(os.listdir(timing_dir))
    for category in ['origin', 'instr']:
        for n in n_dirs:
            print(category, "execs", n)
            exec_path = os.path.join(timing_dir, f'{n}/{category}/data/exec.txt')
            if os.path.exists(exec_path):
                exec_means = np.mean(calculate_mean_from_file(exec_path))
                print(exec_means)
                results[category]['execs/s'].extend([exec_means])
        
        for n in n_dirs:
            time_path = os.path.join(timing_dir, f'{n}/{category}/data/time.txt')
            print(category, "times", n, "path: ", time_path)
            if os.path.exists(time_path):
                time_means = np.mean(calculate_mean_from_file(time_path))
                print(time_means)
                results[category]['time'].extend([time_means])

    with open(output_file, 'w') as json_file:
        json.dump(results, json_file, indent=4)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Calculate mean values from experiment files and save to JSON.')
    parser.add_argument('--timing_dir', type=str, required=True, help='Directory containing the experiment results.')
    parser.add_argument('--output_file', type=str, required=True, help='Output JSON file.')
    args = parser.parse_args()
    
    main(args.timing_dir, args.output_file)
