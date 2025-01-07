#!/bin/zsh

OPENSSL_DIR=/home/debian/Skipper/experimental_stands/openssl/openssl
LOG_FILE=${OPENSSL_DIR}/../make.logs

cd $OPENSSL_DIR
make CC=clang VFP=1 -j $(nproc) > $LOG_FILE 2>&1