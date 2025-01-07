#!/bin/zsh

OPENSSL_DIR=/home/debian/Skipper/experimental_stands/openssl/openssl
FUZZ_DIR=${OPENSSL_DIR}/fuzz
CORPORA_DIR=${FUZZ_DIR}/corpora

cd $OPENSSL_DIR
if [ -n "$1" ]; then
    if [ "$1" = "--" ]; then
        shift
        ./fuzz/bignum $@ -artifact_prefix=${CORPORA_DIR}/bignum-crash/ ${CORPORA_DIR}/bignum ${CORPORA_DIR}/bignum-crash
    else
        echo "bad args. first arg must be <--> or none args must be passed."
    fi
else
    python3 fuzz/helper.py bignum
fi