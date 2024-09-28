#ifndef GET_ALL_SYMBOLS_header
#define GET_ALL_SYMBOLS_header

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <map>

// #include <type_traits>
#include "dr_api.h"
#include "drreg.h"
#include "drsyms.h"
#include "drmgr.h"
#include "drx.h"
#include "dr_modules.h"
#include <string.h>
// #include <libelf.h>
// #include <gelf.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

bool get_all_symbols_callback(drsym_info_t *info, drsym_error_t status, void *data) {
    std::set<std::string> * d = (std::set<std::string>*) data;
    if (info->name) {
        d->insert(info->name);
    }
    return true;
}

std::vector<std::string> 
get_all_symbols(std::string module_name, std::string module_path) {
    if (module_name.empty() || module_path.empty()) {
        perror("NULL module name has been passed! : get_all_symbols\n");
        throw std::runtime_error("NULL module name has been passed! : get_all_symbols");
    }

    module_data_t * module = dr_lookup_module_by_name(module_name.c_str());
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\" : get_all_symbols\n", module_name.c_str());
    }

    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;
   
    error = drsym_get_module_debug_kind(module_path.c_str(), &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind() : get_all_symbols\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw std::runtime_error("error in drsym_get_module_debug_kind() : get_all_symbols");
    } else {
        // printf("kind: %d\n", kind);
    }

    std::set<std::string> data;
    error = drsym_enumerate_symbols_ex( module_path.c_str(),
                                        get_all_symbols_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    auto iterator_ex = dr_symbol_export_iterator_start(module->handle);
    do {
        auto * symbol = dr_symbol_export_iterator_next(iterator_ex);
        data.insert(std::string(symbol->name));
    } while (dr_symbol_export_iterator_hasnext(iterator_ex));
    dr_symbol_export_iterator_stop(iterator_ex);

    auto iterator_im = dr_symbol_import_iterator_start(module->handle, NULL);
    do {
        auto * symbol = dr_symbol_import_iterator_next(iterator_im);
        data.insert(std::string(symbol->name));
    } while (dr_symbol_import_iterator_hasnext(iterator_im));
    dr_symbol_import_iterator_stop(iterator_im);

    drsym_exit();
    dr_free_module_data(module);

    dr_printf("set of symbols to vector transformation...\n");
    std::vector<std::string> res(data.begin(), data.end());

    return res;
}


bool 
get_all_symbols_with_offsets_callback(
                                    drsym_info_t *info, 
                                    drsym_error_t status, 
                                    void *data) 
{
    auto d = (std::map<std::string, generic_func_t>*) data;
    if (info->name) {
        // dr_printf("off: %zu; ", info->start_offs);
        (*d)[info->name] = (generic_func_t) info->start_offs;
    }
    return true;
}


std::map<std::string, generic_func_t> 
get_all_symbols_with_offsets(  
                            std::string module_name, 
                            std::string module_path,  
                            bool use_pattern=false) {
    if (module_name.empty() || module_path.empty()) {
        perror("should not be any empty args have been passed in this function!\n");
        throw std::runtime_error("empty arg has been passed!");
    }

    module_data_t * module = dr_lookup_module_by_name(module_name.c_str());
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\" : get_all_symbols\n", module_name.c_str());
    } else {
        dr_printf("module_path: %s\n", module->full_path);
    }

    if (drsym_init(NULL) != DRSYM_SUCCESS) {
        dr_printf("init dr_sym error. exception throwen\n");
        throw std::runtime_error("cannot init dr_mgr");
    }
    drsym_error_t error;
    drsym_debug_kind_t kind;
   
    error = drsym_get_module_debug_kind(module_path.c_str(), &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind() : get_all_symbols\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw std::runtime_error("[EXEPTION]: error in drsym_get_module_debug_kind() : get_all_symbols");
    } else {
        // printf("kind: %d\n", kind);
    }
    size_t base = (size_t) module->start;

    //#################### начало работы #####################

    // вытаскиваем все пары символ-адрес
    std::map<std::string, generic_func_t> data;
    error = drsym_enumerate_symbols_ex( module_path.c_str(),
                                        get_all_symbols_with_offsets_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    // прибывляем отступ модуля
    for (auto & symbol : data) {
        symbol.second = (generic_func_t) ((size_t) symbol.second + base);
    }

    drsym_exit();
    dr_free_module_data(module);

    return data;
}


#endif // GET_ALL_SYMBOLS_header
