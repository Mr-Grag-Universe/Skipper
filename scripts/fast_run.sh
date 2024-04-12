#!/bin/zsh

./scripts/build_client.sh
./scripts/build_fuzz.sh
clear
./DynamoRIO-Linux-10.0.19672/bin64/drrun -c bin/libclient.so -- bin/fuzz_app