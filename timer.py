import sys
import subprocess
import time

if len(sys.argv) < 3:
    print("Usage: python3 script.py <n> <command> [args...]")
    sys.exit(1)

n = int(sys.argv[1])
command = sys.argv[2:]

total_time = 0.0

for _ in range(n):
    start_time = time.time()
    subprocess.run(command)
    end_time = time.time()
    total_time += end_time - start_time

print(f"Total time for {n} executions: {total_time:.6f} seconds")
print(f"Mean time: {(total_time/n):.6f} seconds")