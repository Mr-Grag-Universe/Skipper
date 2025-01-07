#!/bin/zsh

OPENSSL_DIR=/home/debian/Skipper/experimental_stands/openssl/openssl
CONF=${OPENSSL_DIR}/config

./$CONF enable-fuzz-libfuzzer \
        --with-fuzzer-include=$PATH_TO_LIBFUZZER_DIR \
        --with-fuzzer-lib=$PATH_TO_LIBFUZZER \
        -DPEDANTIC linux-x86_64-clang enable-asan no-ubsan no-shared \
        -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
        -fsanitize=fuzzer-no-link \
        enable-ec_nistp_64_gcc_128 -fno-sanitize=alignment \
        enable-weak-ssl-ciphers enable-rc5 enable-md2 \
        enable-ssl3 enable-ssl3-method enable-nextprotoneg \
        -lstdc++ --debug