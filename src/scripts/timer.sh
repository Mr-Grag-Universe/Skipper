#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <n> <command> [args...]"
    exit 1
fi

n=$1
shift
command="$@"

total_time=0

for (( i=1; i<=n; i++ ))
do
    start_time=$(date +%s.%N)
    $command &> /dev/null
    end_time=$(date +%s.%N)
    elapsed=$(echo "$end_time - $start_time" | bc)
    total_time=$(echo "$total_time + $elapsed" | bc)
    
    if (( i % 10 == 0 )); then
        echo "Completed $i iterations"
    fi
done

average_time=$(echo "scale=6; $total_time / $n" | bc)

echo "Total time for $n executions: $total_time seconds"
echo "Average time per execution: $average_time seconds"
