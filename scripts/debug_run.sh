#!/bin/zsh

./scripts/build_client.sh
./scripts/build_fuzz.sh
clear
./DynamoRIO-Linux-10.0.19672/bin64/drrun -debug -loglevel 4 -logdir /tmp/dynamorio/ -c bin/libclient.so -- bin/fuzz_app
