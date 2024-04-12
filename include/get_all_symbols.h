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
get_all_symbols(const char * module_name) {
    if (module_name == NULL) {
        perror("NULL module name has been passed!\n");
        throw std::runtime_error("NULL module name has been passed!");
    }

    module_data_t * module = dr_lookup_module_by_name(module_name);
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\"", module_name);
    }

    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;
   
    error = drsym_get_module_debug_kind(module_name, &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind()\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw std::runtime_error("error in drsym_get_module_debug_kind()");
    } else {
        // printf("kind: %d\n", kind);
    }

    std::vector<std::string> data;
    error = drsym_enumerate_symbols_ex( module_name,
                                        get_all_symbols_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    drsym_exit();
    dr_free_module_data(module);

    return data;
}

#endif // GET_ALL_SYMBOLS_header