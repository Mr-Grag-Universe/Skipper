#ifndef GET_ALL_SYMBOLS_header
#define GET_ALL_SYMBOLS_header

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
// #include <type_traits>
#include "dr_api.h"
#include "drreg.h"
#include "drsyms.h"
#include "drmgr.h"
#include "drx.h"
#include "dr_modules.h"
#include <string.h>
#include <libelf.h>
#include <gelf.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector>

bool get_all_symbols_callback(drsym_info_t *info, drsym_error_t status, void *data) {
    std::vector<std::string> * d = (std::vector<std::string>*) data;
    if (info->name) {
        d->push_back(info->name);
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
        fprintf(stderr, "cannot load module with name \"%s\" : get_all_symbols", module_name.c_str());
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

    std::vector<std::string> data;
    error = drsym_enumerate_symbols_ex( module_path.c_str(),
                                        get_all_symbols_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    drsym_exit();
    dr_free_module_data(module);

    return data;
}

#endif // GET_ALL_SYMBOLS_header