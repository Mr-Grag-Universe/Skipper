#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
// #include <openssl/des.h>
#include <thread>
#include <chrono>
#include <filesystem>
#include "libbn256.h"

using namespace std::chrono_literals;

#ifdef __linux__
__attribute__((section("__libfuzzer_extra_counters")))
#endif
uint8_t extra_counters[65*512];

namespace fs = std::filesystem;
std::string constructFilePath(const std::string& directory, const std::string& filename) {
    // Создаем объект пути для директории
    fs::path dirPath(directory);
    // Добавляем к пути название файла
    fs::path filePath = dirPath / filename;
    // Возвращаем полный путь в виде строки
    return filePath.string();
}

// Функция для преобразования байта в шестнадцатеричное представление
std::string byteToHex(unsigned char byte) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
    return oss.str();
}

// Функция для сохранения массива байт в файл с указанным названием
void saveBytesToFile(const uint8_t * data, size_t size, std::string path) {
    // Проверка, состоит ли массив только из нулевых байт
    bool isAllZeros = true;
    for (size_t i = 0; i < size; ++i) {
        if (data[i] != 0) {
            isAllZeros = false;
            break;
        }
    }

    // Формирование имени файла
    std::string filename;
    if (isAllZeros) {
        filename = "null";
    } else {
        std::ostringstream filenameStream;
        for (size_t i = 0; i < size; ++i) {
            filenameStream << byteToHex(data[i]);
        }
        filename = filenameStream.str();
    }

    // Запись массива байт в файл
    std::ofstream outFile;
    outFile.open(constructFilePath(path, filename));
    if (!outFile || !outFile.is_open()) {
        std::cerr << "Error: Could not create file " << filename << std::endl;
        return;
    }
    // outFile.write((const char *) data, size);
    outFile.close();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    
    // std::cout << "###############################################" << std::endl;
    // std::cout << "##                  new round                ##" << std::endl;
    // std::cout << "###############################################" << std::endl;

    // std::cout << "thread number: " << std::dec << std::this_thread::get_id() << std::endl;
    // std::cout << "extra counters offset: " << (size_t) extra_counters << std::endl;
    // extra_counters[Data[0]] = 1;
    // fuzz_des(len, Data);

    if (data == NULL) {
        std::cout << "null data has been provided" << std::endl;
        return 0;
    }
    
    // std::cout << "len: " << size << std::endl;
    // std::cout << "DATA: " << (size_t) data << std::endl;
    if (size < 32*2) {
        // std::this_thread::sleep_for(1s);
        return 0;
    }

    // std::cout << "experiment processing..." << std::endl;
    // std::this_thread::sleep_for(0.5s);
    
    int a = gfP_Unmarshal((GoInt) -1, {(void *) data, 32, 32});
    if (a < 0) {
        ClearAll();
        return 0;
    }
    int b = gfP_Unmarshal((GoInt) -1, {(void *) (data+32), 32, 32});
    if (b < 0) {
        ClearAll();
        return 0;
    }
    // std::cout << "a, b created" << std::endl;
    // PrintOC();

    // saveBytesToFile(data, 32*2, "out/corpus");
    // std::cout << "data saved!" << std::endl;

    int a_sq  = gfP_newGFP(0);
    int b_sq  = gfP_newGFP(0);

    int a_m_b = gfP_newGFP(0);
    int a_p_b = gfP_newGFP(0);

    int X_1   = gfP_newGFP(0);
    int X_2   = gfP_newGFP(0);

    // std::cout << "some intermediete result vars created" << std::endl;
    // PrintOC();

    gfP_Add(a_p_b, a, b);
    gfP_Sub(a_m_b, a, b);
    gfP_Mul(X_1, a_m_b, a_p_b);
    // std::cout << "first calculation complete!" << std::endl;
    // PrintOC();

    gfP_Mul(a_sq, a, a);
    gfP_Mul(b_sq, b, b);
    gfP_Sub(X_2, a_sq, b_sq);
    // std::cout << "second calculation complete!" << std::endl;
    // PrintOC();

    if (not gfP_Equal(X_1, X_2)) {
        std::cout << "gfP are not equal!" << std::endl;
        throw std::runtime_error("gfP are not equal!");
    }

    ClearAll();

    // std::cout << "experiment finished" << std::endl;
    // std::this_thread::sleep_for(1s);

    // std::cout << "extra counters data: \n";
    // for (size_t i = 0; i < 512; ++i) {
    //     bool add_is_0 = true;
    //     for (size_t j = 0; j < 65; ++j) {
    //         if (extra_counters[i*65+j]) {
    //             add_is_0 = false;
    //             break;
    //         }
    //     }
    //     if (add_is_0) {
    //         continue;
    //     }
    //     std::cout << "ind: " << i << " : ";
    //     for (size_t j = 0; j < 65; ++j) {
    //         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(extra_counters[i*65+j]);
    //     }
    //     std::cout << std::endl;
    // }

    // std::cout << "###############################################" << std::endl;
    // std::cout << "##               round finished              ##" << std::endl;
    // std::cout << "###############################################" << std::endl;

    // std::this_thread::sleep_for(1s);
    return 0;
}
