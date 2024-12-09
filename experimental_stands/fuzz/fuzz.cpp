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
// #include "libbn256.h"

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

#include <iostream>

// extern "C" unsigned long long my_export_asm_factorial(int n);

void my_asm_factorial(int a, int b, int *result) {
    // unsigned long long result;
    // __asm__ (
    //     // "mov %1, %%rdi;"      // Загружаем n в регистр rdi
    //     // "mov $1, %%rax;"      // Инициализируем результат (rax) как 1
    //     // "cmp $1, %%rdi;"      // Сравниваем n с 1
    //     // "jle end;"             // Если n <= 1, переходим к end"
    //     "loop_start:"          // Начало цикла"
    //     "mul %%rdi;"           // Умножаем rax на rdi"
    //     "dec %%rdi;"           // Уменьшаем rdi на 1"
    //     // "cmp $1, %%rdi;"       // Сравниваем n с 1"
    //     "jg loop_start;"       // Если n > 1, продолжаем цикл"
    //     "end:"                 // Конец"
    //     "mov %%rax, %0;"      // Сохраняем результат в переменную result
    //     : "=r" (result)       // Выходные операнды
    //     : "r" (n)             // Входные операнды
    //     : "%rax", "%rdi"     // Изменяемые регистры
    // );
    // int result;
    // Используем встроенный ассемблер
    __asm__(
        "push %%rax;"
        "mov $5, %%eax;"
        "cmp %1, %%eax;"      // Сравниваем a с 10
        "pop %%rax;"

        "jle end;"          // Если a <= 10, переходим к метке end
        "add %2, %1;"    // Складываем a и b
        "mov %1, %0;" // Сохраняем результат в переменную result
        "jmp finish;"        // Переходим к метке finish
        "end:\n"
        "mov %1, %0;" // Если a <= 10, просто сохраняем a в result
        "finish:\n"
        : "=r" (*result) // Выходной аргумент
        : "r" (a), "r" (b) // Входные аргументы
    );

    // __asm__ __volatile__ ( "movl %1, %%eax;"
    //                       "movl %2, %%ebx;"
    //                       "CONTD: cmpl $0, %%ebx;"
    //                       "je DONE;"
    //                       "xorl %%edx, %%edx;"
    //                       "idivl %%ebx;"
    //                       "movl %%ebx, %%eax;"
    //                       "movl %%edx, %%ebx;"
    //                       "jmp CONTD;"
    //                       "DONE: movl %%eax, %0;" : "=g" (result) : "g" (a), "g" (b)
    // );

    // return result;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    
    // std::cout << "###############################################" << std::endl;
    // std::cout << "##                  new round                ##" << std::endl;
    // std::cout << "###############################################" << std::endl;

    // std::cout << "thread number: " << std::dec << std::this_thread::get_id() << std::endl;
    // std::cout << "extra counters offset: " << (size_t) extra_counters << std::endl;
    // extra_counters[Data[0]] = 1;
    // fuzz_des(len, Data);

    // my_export_asm_factorial(1);

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

    std::cout << "experiment processing..." << std::endl;
    std::this_thread::sleep_for(0.5s);
    int res;
    my_asm_factorial(100, 23, &res);

    // int a = gfP_Unmarshal((GoInt) -1, {(void *) data, 32, 32});
    // if (a < 0) {
    //     ClearAll();
    //     return 0;
    // }
    // int b = gfP_Unmarshal((GoInt) -1, {(void *) (data+32), 32, 32});
    // if (b < 0) {
    //     ClearAll();
    //     return 0;
    // }
    // std::cout << "a, b created" << std::endl;
    // PrintOC();

    // saveBytesToFile(data, 32*2, "out/corpus");
    // std::cout << "data saved!" << std::endl;

    // int a_sq  = gfP_newGFP(0);
    // int b_sq  = gfP_newGFP(0);

    // int a_m_b = gfP_newGFP(0);
    // int a_p_b = gfP_newGFP(0);

    // int X_1   = gfP_newGFP(0);
    // int X_2   = gfP_newGFP(0);

    // std::cout << "some intermediete result vars created" << std::endl;
    // PrintOC();

    // gfP_Add(a_p_b, a, b);
    // gfP_Sub(a_m_b, a, b);
    // gfP_Mul(X_1, a_m_b, a_p_b);
    // std::cout << "first calculation complete!" << std::endl;
    // PrintOC();

    // gfP_Mul(a_sq, a, a);
    // gfP_Mul(b_sq, b, b);
    // gfP_Sub(X_2, a_sq, b_sq);
    // std::cout << "second calculation complete!" << std::endl;
    // PrintOC();

    // if (not gfP_Equal(X_1, X_2)) {
    //     std::cout << "gfP are not equal!" << std::endl;
    //     throw std::runtime_error("gfP are not equal!");
    // }

    // ClearAll();

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
