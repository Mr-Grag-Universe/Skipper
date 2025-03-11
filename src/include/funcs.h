#ifndef FUNCS_DR_header
#define FUNCS_DR_header

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "dr_api.h"
#include "drreg.h"
#include "drsyms.h"
#include "drmgr.h"
#include "drx.h"
#include "dr_modules.h"

#include "types.h"

// используется, чтобы достать отступ extra_counters
size_t get_symbol_offset(std::string module_name, std::string module_path, std::string symbol_name) {
    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;
    
    error = drsym_get_module_debug_kind(module_path.c_str(), &kind);
    if (error != DRSYM_SUCCESS) {
        // main_logger.log_error("error in drsym_get_module_debug_kind() : get_symbol_offset : kind error");
        dr_fprintf(STDERR, "ERROR: error in drsym_get_module_debug_kind() : get_symbol_offset\n");
        return 0;
    } else {
        // printf("kind: %d\n", kind);
    }

    size_t offset = 0;
    error = drsym_lookup_symbol(module_path.c_str(), 
                                symbol_name.c_str(),
                                &offset,
                                DRSYM_DEMANGLE_FULL);
    if (error != DRSYM_SUCCESS) {
        // main_logger.log_error("error in drsym_lookup_symbol() : get_symbol_offset");
        dr_fprintf(STDERR, "ERROR: error in drsym_lookup_symbol() : get_symbol_offset\n");
        return 0;
    } else {
        // printf("offset: %d\n", offset);
    }
    
    drsym_exit();

    return offset;
}

// используется
std::vector<ModuleInfo> get_all_modules() {
    auto iterator = dr_module_iterator_start();
    std::vector<ModuleInfo> modules;
    while (dr_module_iterator_hasnext(iterator)) {
        auto * module = dr_module_iterator_next(iterator);
        modules.push_back({dr_module_preferred_name(module), module->full_path});
        dr_free_module_data(module);
    }
    dr_module_iterator_stop(iterator);
    return modules;
}

// используется
std::vector <std::string> get_modules_names() {
    auto modules = get_all_modules();
    std::vector <std::string> modules_names;
    for (auto module : modules) {
        auto mn = module.name;
        modules_names.push_back(mn);
    }
    return modules_names;
}

std::string int_to_hex(int my_integer) {
    std::stringstream sstream;
    sstream << std::hex << my_integer;
    std::string result = sstream.str();
    return result;
}

std::string get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

#endif // FUNCS_DR_header
