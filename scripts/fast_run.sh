#!/bin/zsh

./scripts/build_client.sh
./scripts/build_fuzz.sh
clear
# cd /home/stepan/Desktop/fedora_files/fuzzing/Skipper/Skipper_1.0/bin
./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app