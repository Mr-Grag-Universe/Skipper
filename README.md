# Skipper

This is my scientific work.

Skipper_1.0 is a working stable version of my fuzzing client for Libfuzzer from LLVM and Google, which can improve stability and speed of **crypto library fuzzing**.

It uses the **DynamoRIO** framework to analyze and instrument code.

It still needs to be tuned by hands to work in your case, but most common settings can be set in the special config file, which you can find in the `input` folder.

## Functionality

For now, my client does:
1. Find `__libfuzzer_extra_counters` bounds
2. Find bounds of inspection functions
    * But you can use the default preset bounds if you want
3. Code instrumentation
    * Trace overflow in the destination register
    * Trace only chosen opcodes
4. Logging

## Installation

I programmed this client on Fedora Linux (versions 38-40) so it's the perfect system to try this tool. Generally, the client should work on all the most common distributions of **GNU/Linux**.

The common way to run fuzzing with the client should be:
1. Build the fuzzing program with Libfuzzer \
    *In my case*:
    ``` bash
    $ ./scripts/build_fuzzer.sh
    ```
2. Configure the client with the `input/settings.json` file and maybe something in the codebase of the client
3. Build the client
    ``` bash
    $ ./scripts/build_client.sh
    ```
4. Run fuzzing under the DynamoRIO client
    *For example*:
    ```
    ./DynamoRIO-Linux-10.0.19672/bin64/drrun -c ./bin/libclient.so -- ./bin/fuzz_app -max_len=64 -len_control=1 out/corpus
    ```
    where `libclient.so` is the built client module, and `fuzz_app` is the Libfuzzer-built program.