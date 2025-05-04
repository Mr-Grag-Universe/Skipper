# Skipper

<!-- ![Icon](demo/icon.png) -->
<img src=demo/icon.png width=200>

This is my scientific work.

Skipper_1.0 is a working stable version of my fuzzing client for Libfuzzer from LLVM and Google, which can improve stability and speed of **crypto library fuzzing**.

It uses the **DynamoRIO** framework to analyze and instrument code.

It still needs to be tuned by hands to work in your case, but most common settings can be set in the special config file, which you can find in the `input` folder.

## Functionality

For now, my client can:
<!-- 1. Find `__libfuzzer_extra_counters` bounds
2. Find bounds of inspection functions
    * But you can use the default preset bounds if you want
3. Code instrumentation
    * Trace overflow in the destination register
    * Trace only chosen opcodes
4. Logging -->

+ Trace whole functions (each instruction of it), provided in `.json` settings file.
+ Trace all ASM instructions in inline-asm parts of code.
+ Can be tuned with `.json` config file.

## Installation

For development I used **Fedora Linux 38** and **Debian Linux 12 (bookworm) KVM**. So probably nearest **GNU/Linux** distributions are the perfect system to try this tool. 
BUT it's strongly recommended to use the latest version release of DynamoRIO library for your architecture. For example, when I switched to the new Fedora 40 version I faced with the issue, that was been fixing at that moment (some dependency troubles with glibc version). So again, CHECK VERSIONS. Or just use containers =).

To install project clone the repository:
``` bash
$ git clone https://github.com/Mr-Grag-Universe/Skipper.git
```
Go inside and download the latest DynamoRIO release (for example):
``` bash
$ cd Skipper
$ curl -o dynamorio.tar.gz https://github.com/DynamoRIO/dynamorio/releases/download/cronbuild-11.90.20175/DynamoRIO-Linux-11.90.20175.tar.gz
$ mkdir -p dynamorio && tar -xzf dynamorio.tar.gz -C dynamorio --strip-components=1
$ rm dynamorio.tar.gz
```

Just in case you can add `dynamorio` and `dynamorio/bin64` to the PATH.

Compile a client (I used CMake for building).

``` bash
$ cmake --build build -j $(nproc)
```

Graz! It's ready for work.
Next step is to write your `settings.json` config and setup libfuzzer test program.

<!-- 
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
    where `libclient.so` is the built client module, and `fuzz_app` is the Libfuzzer-built program. -->

## Usage

This client can be used as any other DynamoRIO client. 
More options for `drrun` can be found in their official [documentation](https://dynamorio.org/).

Template run:
```bash
$ path/to/drrun [DR options] -c path/to/client.so [client options] -- path/to/fuzz [fuzz program options]
```


### Command line arguments

+ `config` - Path to the configuration file. By default it is `input/settings.json`

### Simple running
``` bash
$ dynamorio/bin64/drrun -c build/bin/libclient.so -- fuzz
```

### Debug run
``` bash
$ dynamorio/bin64/drrun -debug -loglevel 3 -c build/bin/libclient.so -- fuzz
```

## Settings

Here is an example of configuration JSON.
``` json
{
    "tracer" : { // settings for Tracer class object
        "start_location" : 0,
        "trace_size" : -1,
        "trace_area_name" : "__libfuzzer_extra_counters", // will be transformed to start and end symbols
        "use_asm" : true,   // usage of inline-asm instrumentation
        "debug" : true
    },
    "fuzzing" : { // setting for symbols search
        "use_pattern" : true, // use paterns to find function bounds
        "use_default" : true, // use default bounds below without asking user for input
        "inspect_funcs" : [   // list of functions to trace ASM-instructions from
            {
                "module_name" : "bignum", 
                "module_path" : "experimental_stands/openssl/openssl/fuzz/bignum", 
                "func_name"   : "bn_mulx4x_mont",
                "default_address" : {
                    "start" : "0000000000000000",
                    "stop"  : "0000000000000000"
                }
            }
        ],
        "inspect_opcodes" : ["add", "movq", "cmp", "adcx", "adox"],
        "corpus_path" : "out/corpus" // depricated
    },
    "logging" : {
        "log_symbols" : { // logs all symbols in modules
            "enable" : true,
            "path" : "out/logs/symbols.txt"
        },
        "log_fuzzing" : { // logs everything
            "enable" : true,
            "path" : "out/logs/log.txt"
        }
    },
    "debug" : true
}
```

Just copy it and modify as you like.

Don't forget to path the location of `settings.json` for client by command line arguments.

## LLVM-Guards

I developed my own method of ASM-inline code detection for LLVM compilation.

In `LLVM/llvm_pass` you can find code for LLVM-pass modules. To use it you should do next:
+ copy `MySkipperPass.cpp` into `llvm/lib/Transforms/Utils/`
+ copy `MySkipperPass.h` into `llvm/include/llvm/Transforms/Utils`
+ insert `FUNCTION_PASS("skipper", SkipperPass())` into `llvm/lib/Passes/PassRegistry.def`
+ insert `#include "llvm/Transforms/Utils/Skipper.h"` into `llvm/lib/Passes/PassBuilder.cpp`
+ insert 'MySkipperPass.cpp' into `llvm/lib/Transforms/Utils/CMakeLists.txt`

To simplify edit and use `LLVM/llvm_pass/scripts/push_pass.sh`.

Now you should rebuild LLVM.
See LLVM [documentation](https://llvm.org/docs/GettingStarted.html) for more details.

I used these commands to setup my system:
``` bash
$ cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=MinSizeRel -DLLVM_PARALLEL_COMPILE_JOBS=6 -DLLVM_PARALLEL_LINK_JOBS=6 -DLLVM_PARALLEL_TABLEGEN_JOBS=6 -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra;compiler-rt" -DLLVM_INSTALL_UTILS=ON
$ cmake --build build -j $(nproc)
```

Good luck to configure everithing correctly ;-)

If everything ok, you can use custom pass when you build your programs.

