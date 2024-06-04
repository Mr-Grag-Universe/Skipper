#!/bin/zsh

mkdir ./out/timing/1/origin
mkdir ./out/timing/1/origin/data
mkdir ./out/timing/1/instr
mkdir ./out/timing/1/instr/data
cp ./out/timing/1/mul_bmi2_amd64.h
./scripts/build_bn.sh && ./scripts/build_fuzz.sh
python timer.py --n_iter=100 --command="./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/1/origin/data/time.txt
cp out/timing/exec.txt out/timing/1/origin/data/exec.txt
rm -rf crash-*
python timer.py --n_iter=100 --command="./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/1/instr/data/time.txt
cp out/timing/exec.txt out/timing/1/instr/data/exec.txt

mkdir ./out/timing/2/origin
mkdir ./out/timing/2/origin/data
mkdir ./out/timing/2/instr
mkdir ./out/timing/2/instr/data
cp ./out/timing/1/mul_bmi2_amd64.h
./scripts/build_bn.sh && ./scripts/build_fuzz.sh
python timer.py --n_iter=100 --command="./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/2/origin/data/time.txt
cp out/timing/exec.txt out/timing/2/origin/data/exec.txt
rm -rf crash-*
python timer.py --n_iter=100 --command="./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/2/instr/data/time.txt
cp out/timing/exec.txt out/timing/2/instr/data/exec.txt


mkdir ./out/timing/3/origin
mkdir ./out/timing/3/origin/data
mkdir ./out/timing/3/instr
mkdir ./out/timing/3/instr/data
cp ./out/timing/3/mul_bmi2_amd64.h
./scripts/build_bn.sh && ./scripts/build_fuzz.sh
python timer.py --n_iter=100 --command="./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/3/origin/data/time.txt
cp out/timing/exec.txt out/timing/3/origin/data/exec.txt
rm -rf crash-*
python timer.py --n_iter=100 --command="./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app -max_len=64 -len_control=0"
cp out/timing/time.txt out/timing/3/instr/data/time.txt
cp out/timing/exec.txt out/timing/3/instr/data/exec.txt