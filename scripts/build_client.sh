#!/bin/zsh

cd build
cmake ../
make
cp bin/libclient.so ../bin/libclient.so
