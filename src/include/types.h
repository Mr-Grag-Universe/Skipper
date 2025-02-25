
#ifndef MY_TYPES_header
#define MY_TYPES_header

#include <stdint.h>
#include <stddef.h>
#include <cstring>
#include <ostream>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "dr_api.h"


struct sym_info_t {
    std::string file;

    uint64 line;
    size_t line_offs;
    size_t start_offs;
    size_t end_offs;

    int debug_kind;

    std::string name;
    uint type_id = 0;
    uint flags;

    bool ex = false;
    bool imp = true;
    size_t moffs = 0;

    sym_info_t(const char * name, size_t off, bool imp = false) {
        if (name == NULL) {
            throw "error";
        }
        this->ex = false;
        this->imp = imp;
        this->name = std::string(name);
        this->moffs = off;
    }
    sym_info_t(const drsym_info_t * info, bool imp=false) {
        // std::cout << "construct" << std::endl;
        if (!info) {
            perror("NULL drsym_info_t pointer in constructor!\n");
            throw "error";
        }
        this->ex = true;
        this->imp = imp;
        this->name = std::string(info->name, info->name_size);
        this->type_id = info->type_id;
        this->flags = info->flags;

        this->file = std::string(info->file, info->file_size);

        this->line = info->line;
        this->line_offs = info->line_offs;
        this->start_offs = info->start_offs;
        this->end_offs = info->end_offs;
    }

    ~sym_info_t() {}

    sym_info_t(const sym_info_t & sym) {
        // std::cout << "copy" << sym.name << std::endl;
        this->name = sym.name;
        this->type_id = sym.type_id;
        this->flags = sym.flags;

        this->file = sym.file;

        this->line = sym.line;
        this->line_offs = sym.line_offs;
        this->start_offs = sym.start_offs;
        this->end_offs = sym.end_offs;

        this->ex = sym.ex;
        this->imp = sym.imp;
    }
};

struct ModuleInfo {
    std::string name;
    std::string path;
};

struct FuncConfig {
    std::string module_name;
    std::string module_path;
    std::pair<size_t, size_t> default_address;
};

static const size_t MAP_SIZE = 1025;
typedef struct thread_data
{
    uint64_t location;
    uint8_t map[MAP_SIZE];
};

#endif // MY_TYPES_header