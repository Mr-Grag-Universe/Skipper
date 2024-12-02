#ifndef FIND_PATTERN_header
#define FIND_PATTERN_header

#include "funcs.h"

struct ExtraData {
    std::string search_pattern;
    std::vector<std::string> names;
    std::vector<size_t> offsets;
};

bool find_func_address_callback(drsym_info_t *info, drsym_error_t status, void *data) {
    ExtraData * d = (ExtraData*) data;
    // printf("symbol: %s\n", name);
    if (info->name) {
        if (strstr(info->name, d->search_pattern.c_str())) {
            printf("symbol: %s\n", info->name);
            printf("start_offs: %zu\n", info->start_offs);
            printf("end_offs: %zu\n", info->end_offs);
            d->names.push_back(std::string(info->name));
            d->offsets.push_back(info->start_offs);
        }
    }
    return true;
}

std::pair<std::vector<std::string>, std::vector<size_t>> 
get_pattern_addresses(const char * module_name, const char * pattern) {
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

    ExtraData data = ExtraData();
    data.search_pattern = pattern;
    error = drsym_enumerate_symbols_ex( module_name,
                                        find_func_address_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    drsym_exit();
    dr_free_module_data(module);

    return std::make_pair(data.names, data.offsets);
}

#endif // FIND_PATTERN_header