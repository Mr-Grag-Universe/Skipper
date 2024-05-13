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
    
    std::cout << "len: " << size << std::endl;
    std::cout << "DATA: " << (size_t) data << std::endl;
    if (size < 16) {
        std::this_thread::sleep_for(1s);
        return 0;
    }

    std::cout << "experiment processing..." << std::endl;
    std::this_thread::sleep_for(0.5s);
    
    size_t a_int = *((size_t*) (data));
    size_t b_int = *((size_t*) (data+8));
    int a = gfP_newGFP(a_int);
    int b = gfP_newGFP(b_int);
    std::cout << "a, b created" << std::endl;
    PrintOC();

    int a_sq  = gfP_newGFP(0);
    int b_sq  = gfP_newGFP(0);

    int a_m_b = gfP_newGFP(0);
    int a_p_b = gfP_newGFP(0);

    int X_1   = gfP_newGFP(0);
    int X_2   = gfP_newGFP(0);

    std::cout << "some intermediete result vars created" << std::endl;
    PrintOC();

    gfP_Add(a_p_b, a, b);
    gfP_Sub(a_m_b, a, b);
    gfP_Mul(X_1, a_m_b, a_p_b);
    std::cout << "first calculation complete!" << std::endl;
    PrintOC();

    gfP_Mul(a_sq, a, a);
    gfP_Mul(b_sq, b, b);
    gfP_Sub(X_2, a_sq, b_sq);
    std::cout << "second calculation complete!" << std::endl;
    PrintOC();

    if (not gfP_Equal(X_1, X_2)) {
        std::cout << "gfP are not equal!" << std::endl;
        throw std::runtime_error("gfP are not equal!");
    }

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
