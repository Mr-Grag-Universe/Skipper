
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

using namespace std::chrono_literals;

#ifdef __linux__
__attribute__((section("__libfuzzer_extra_counters")))
#endif
uint8_t extra_counters[256];

unsigned char * generate_des_key() {
    int i = 0, j = 0;
    DES_cblock cb;
    DES_key_schedule ks;

    unsigned char * key = (unsigned char *) malloc(DES_SCHEDULE_SZ * 3);
    if (!key) {
        return NULL;
    }

    for(; i < 3; i++) {
        DES_random_key(&cb);
        if((j = DES_set_key_checked(&cb, &ks)) != 0) {
            free(key);
            return NULL;
        }
        memmove(key + i*DES_SCHEDULE_SZ, (unsigned char *)&ks, DES_SCHEDULE_SZ);
    }

    return key;
}

void fuzz_des(size_t len, const uint8_t * Data) {
    unsigned char * key = generate_des_key();
    unsigned char * ivec = (unsigned char *) calloc(len, 1);
    unsigned char * res = (unsigned char *) calloc(len, 1);
    int num;
    DES_ede3_ofb64_encrypt( Data, res, len, 
                            (DES_key_schedule*) key, 
                            (DES_key_schedule *)key+DES_SCHEDULE_SZ, 
                            (DES_key_schedule *)key+DES_SCHEDULE_SZ*2, 
                            (DES_cblock*) ivec, &num);
    free(ivec);
    free(res);
    free(key);
}

extern "C" void _asm_sort(uint8_t * arr, size_t size);

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

    uint8_t * arr = (uint8_t *) malloc(size * sizeof(uint8_t));
    if (arr == NULL) {
        return 0;
    }

    memcpy(arr, data, size);
    std::cout << "_asm_sort processing..." << std::endl;
    std::this_thread::sleep_for(0.5s);
    _asm_sort(arr, size);
    std::cout << "_asm_sort finished" << std::endl;
    std::this_thread::sleep_for(1s);
    free(arr);

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
