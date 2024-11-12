#!/bin/zsh

./scripts/build_client.sh
./scripts/build_fuzz.sh
clear
./DynamoRIO-Linux/bin64/drrun -debug -loglevel 4 -logdir out/dynamorio/ -c bin/libclient.so -- bin/fuzz_app -max_len=64 -len_control=1
