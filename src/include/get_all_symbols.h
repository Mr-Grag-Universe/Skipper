#ifndef GET_ALL_SYMBOLS_header
#define GET_ALL_SYMBOLS_header

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <map>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

#include "dr_api.h"
#include "drreg.h"
#include "drsyms.h"
#include "drmgr.h"
#include "drx.h"
#include "dr_modules.h"

bool get_all_symbols_with_offsets_callback(
    drsym_info_t *info,
    drsym_error_t status,
    void *data)
{
    auto d = (std::map<std::string, generic_func_t> *)data;
    if (info->name)
    {
        // dr_printf("off: %zu; ", info->start_offs);
        (*d)[info->name] = (generic_func_t)info->start_offs;
    }
    return true;
}

std::map<std::string, generic_func_t>
get_all_symbols_with_offsets(
    std::string module_name,
    std::string module_path,
    bool use_pattern = false)
{
    if (module_name.empty() || module_path.empty())
    {
        perror("should not be any empty args have been passed in this function!\n");
        throw std::runtime_error("empty arg has been passed!");
    }

    module_data_t *module = dr_lookup_module_by_name(module_name.c_str());
    if (module == NULL)
    {
        fprintf(stderr, "cannot load module with name \"%s\" : get_all_symbols\n", module_name.c_str());
    }
    else
    {
        dr_printf("module_path: %s\n", module->full_path);
    }

    if (drsym_init(NULL) != DRSYM_SUCCESS)
    {
        dr_printf("init dr_sym error. exception throwen\n");
        throw std::runtime_error("cannot init dr_mgr");
    }
    drsym_error_t error;
    drsym_debug_kind_t kind;

    error = drsym_get_module_debug_kind(module_path.c_str(), &kind);
    if (error != DRSYM_SUCCESS)
    {
        perror("error in drsym_get_module_debug_kind() : get_all_symbols\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw std::runtime_error("[EXEPTION]: error in drsym_get_module_debug_kind() : get_all_symbols");
    }
    else
    {
        // printf("kind: %d\n", kind);
    }
    size_t base = (size_t)module->start;

    // #################### начало работы #####################

    // вытаскиваем все пары символ-адрес
    std::map<std::string, generic_func_t> data;
    error = drsym_enumerate_symbols_ex(module_path.c_str(),
                                       get_all_symbols_with_offsets_callback,
                                       sizeof(drsym_info_t),
                                       &data,
                                       DRSYM_DEMANGLE_FULL);

    // прибывляем отступ модуля
    for (auto &symbol : data)
    {
        symbol.second = (generic_func_t)((size_t)symbol.second + base);
    }

    drsym_exit();
    dr_free_module_data(module);

    return data;
}

#endif // GET_ALL_SYMBOLS_header
