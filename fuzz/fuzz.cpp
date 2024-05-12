#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/des.h>
#include <thread>
#include <chrono>
#include "libbn256.h"

using namespace std::chrono_literals;

#ifdef __linux__
__attribute__((section("__libfuzzer_extra_counters")))
#endif
uint8_t extra_counters[256];

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    
    std::cout << "###############################################" << std::endl;
    std::cout << "##                  new round                ##" << std::endl;
    std::cout << "###############################################" << std::endl;

    std::cout << "thread number: " << std::dec << std::this_thread::get_id() << std::endl;
    std::cout << "extra counters offset: " << (size_t) extra_counters << std::endl;
    // extra_counters[Data[0]] = 1;
    // fuzz_des(len, Data);

    if (data == NULL) {
        std::cout << "null data has been provided" << std::endl;
        return 0;
    }
    if (size % 200 == 0) {
        size = 1;
    }
    size = size % 200;
    std::cout << "len: " << size << std::endl;
    std::cout << "DATA: " << (size_t) data << std::endl;

    std::cout << "experiment processing..." << std::endl;
    std::this_thread::sleep_for(0.5s);
    
    PrintOC();
    int i = New_G1(3);
    PrintOC();
    int j = New_G1(7);
    PrintOC();
    std::cout << i << " " << j << std::endl;

    int sum_ind = Add_G1(-1, i, j);

    ClearAll();

    std::cout << "experiment finished" << std::endl;
    std::this_thread::sleep_for(1s);

    std::cout << "extra counters data: ";
    for (size_t i = 0; i < 256; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(extra_counters[i]);
    } std::cout << std::endl;

    std::cout << "###############################################" << std::endl;
    std::cout << "##               round finished              ##" << std::endl;
    std::cout << "###############################################" << std::endl;

    std::this_thread::sleep_for(1s);
    return 0;
}
