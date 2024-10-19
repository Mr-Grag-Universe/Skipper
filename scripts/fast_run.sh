#!/bin/zsh

./scripts/build_client.sh
./scripts/build_fuzz.sh
clear
# cd /home/stepan/Desktop/fedora_files/fuzzing/Skipper/Skipper_1.0/bin
./DynamoRIO_Linux_10/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app -max_len=64 -len_control=1 out/corpus # out/corpus # -jobs=4
