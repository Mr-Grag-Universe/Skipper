#!/bin/zsh

cd /home/stepan/Desktop/fedora_files/fuzzing/Skipper/Skipper_1.0/bn256
go build -buildmode=c-shared -o libbn256.so -v -x -work \
bn256_fast.go bn256_handlers.go bn256_curve_point.go bn256_G.go bn256_gfp.go
go build -buildmode=c-archive -o libbn256.a -v -x -work \
bn256_fast.go bn256_handlers.go bn256_curve_point.go bn256_G.go bn256_gfp.go
cp libbn256.so ../bin/libbn256.so
cp libbn256.a ../bin/libbn256.a
cp libbn256.h ../fuzz/libbn256.h