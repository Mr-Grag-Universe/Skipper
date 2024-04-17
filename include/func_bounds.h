#ifndef FIND_FUNC_BOUNDS_header
#define FIND_FUNC_BOUNDS_header

#include <algorithm>

#include "dr_tools.h"
#include "drsyms.h"
#include "drmgr.h"
#include "find_pattern.h"
#include "get_all_symbols.h"

bool get_func_bounds_callback(drsym_info_t *info, drsym_error_t status, void *data) {
    if (info != NULL && data != NULL) {
        // if (status != DRSYM_SUCCESS) {
        //      printf("line shit is not success!\n");
        // }
        auto * d = (std::vector<std::pair<size_t, std::string>> *) data;
        d->push_back(std::make_pair(info->start_offs, info->name));
    }
    return true;
}

std::pair<size_t, size_t> get_func_bounds(std::string module_name, std::string func_name) {
    if (module_name.empty()) {
        perror("empty module name has been passed!\n");
        throw std::runtime_error("empty module name has been passed!");
    }

    module_data_t * module = dr_lookup_module_by_name(module_name.c_str());
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\"", module_name.c_str());
    }

    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;

    error = drsym_get_module_debug_kind(module_name.c_str(), &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind()\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw std::runtime_error("error in drsym_get_module_debug_kind()");
    } else {
        // printf("kind: %d\n", kind);
    }

    std::vector<std::pair<size_t, std::string>> data;
    error = drsym_enumerate_symbols_ex( module_name.c_str(),
                                        get_func_bounds_callback,
                                        sizeof(drsym_info_t),
                                        &data,
                                        DRSYM_DEMANGLE_FULL);

    drsym_exit();
    dr_free_module_data(module);

    // сортируем полученные адреса и ищем среди них нашу функцию
    std::sort(data.begin(), data.end());
    auto iter = std::find_if(data.begin(), data.end(), [&func_name](const auto x){
        return func_name == std::string(x.second);
    });
    if (iter == data.end()) {
        throw std::invalid_argument("there is not such func name here");
    }

    if (iter + 1 != data.end()) {
        printf("find complete!\nnext_name: %s\n", (iter+1)->second.c_str());
        return std::make_pair(iter->first, (iter+1)->first);
    }

    return std::make_pair(iter->first, -1);
}

/*
#####################################################################
                        with get_proc_address
#####################################################################
*/

std::pair<generic_func_t, generic_func_t> get_func_bounds_gpa(std::string module_name, std::string module_path, std::string func_name) {
    if (module_name.empty()) {
        perror("empty module name has been passed!\n");
        throw std::runtime_error("empty module name has been passed!");
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

    auto func_names = get_all_symbols(module_name.c_str(), module_path.c_str());
    std::vector<std::pair<generic_func_t, std::string>> funcs;
    for (auto & func_name : func_names) {
        funcs.push_back(std::make_pair(dr_get_proc_address(module->handle, func_name.c_str()), func_name));
    }

    drsym_exit();
    dr_free_module_data(module);

    // сортируем полученные адреса и ищем среди них нашу функцию
    std::sort(funcs.begin(), funcs.end());
    auto iter = std::find_if(funcs.begin(), funcs.end(), [&func_name](const auto x){
        return func_name == std::string(x.second);
    });
    if (iter == funcs.end()) {
        char message[1024];
        sprintf(message, "there is not func name <%s> here", std::string(iter->second).c_str());
        throw std::invalid_argument(message);
    }

    if (iter + 1 != funcs.end()) {
        printf("find complete!\nnext_name: %s\n", (iter+1)->second.c_str());
        return std::make_pair(iter->first, (iter+1)->first);
    }

    return std::make_pair(iter->first, (generic_func_t)-1);
}

#endif // FIND_FUNC_BOUNDS_header
