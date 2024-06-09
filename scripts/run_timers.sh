#!/bin/zsh

# Constants
EXPERIMENTS=3
MODES=("origin" "instr")
N_ITER=2
ORIGIN_COMMAND="./bin/fuzz_app -max_len=64 -len_control=0"
INSTR_COMMAND="./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- $ORIGIN_COMMAND"
SCRIPT_PATH=./scripts/timer.py

# Functions
run_experiment() {
    local exp_num=$1
    local mode=$2
    mkdir -p "./out/timing/$exp_num/$mode/data"
    if [ "$mode" = "origin" ]; then
        python $SCRIPT_PATH --n_iter=$N_ITER --command="$ORIGIN_COMMAND"
    else
        python $SCRIPT_PATH --n_iter=$N_ITER --command="$INSTR_COMMAND"
    fi
    mv out/timing/time.txt "out/timing/$exp_num/$mode/data/time.txt"
    mv out/timing/exec.txt "out/timing/$exp_num/$mode/data/exec.txt"
}

# Main script
for ((i=1; i<=EXPERIMENTS; i++)); do
    # preparation
    cp "./out/timing/$i/mul_bmi2_amd64.h" "./bn256/cloudflare/mul_bmi2_amd64.h"
    ./scripts/build_bn.sh && ./scripts/build_fuzz.sh

    # execution
    for mode in "${MODES[@]}"; do
        run_experiment $i $mode

        # clear crashes
        if [ "$mode" = "origin" ]; then
            rm -rf crash-*
        fi
    done
done
