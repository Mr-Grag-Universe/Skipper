#ifndef FUNCS_DR_header
#define FUNCS_DR_header

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
// #include <type_traits>
#include <string.h>
#include <libelf.h>
#include <gelf.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include "dr_api.h"
#include "drreg.h"
#include "drsyms.h"
#include "drmgr.h"
#include "drx.h"
#include "dr_modules.h"

#include "types.h"


bool callback_get_all_symbols_1(drsym_info_t * info, drsym_error_t status, void * data) {
    ((std::vector<std::string>*)data)->push_back(info->name);
    return true;
}
bool callback_get_all_symbols_2(const char *name, size_t modoffs, void *data) {
    sym_info_t info(name, modoffs);
    // printf("youuuu\n");
    ((std::vector<sym_info_t>*)data)->push_back(info);
    return true;
}


bool print_module_data(module_data_t * m) {
    if (m == NULL) {
        return false;
    }
    bool err = printf(
            "end                    :   %p\n"
            "entry_point            :   %p\n"
            "flags                  :   %u\n"
            "name                   :   %s\n"
            "full_path              :   %s\n"
            // "file_version           :   %u"
            // "product_version        :   %u"
            // "checksum               :   %u"
            "timestamp              :   %u\n"
            // "module_internal_size   :   %lld"
            "preferred_base         :   %p\n"
            "start                  :   %p\n",
            m->end,
            m->entry_point,
            m->flags,
            dr_module_preferred_name(m),
            m->full_path,
              // m->file_version.version,
            // m->product_version.version,
            // m->checksum,
            m->timestamp,
            // m->module_internal_size,
            m->start);
    return true;
}

size_t get_symbol_offset(std::string module_name, std::string module_path, std::string symbol_name) {
    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;
    
    error = drsym_get_module_debug_kind(module_path.c_str(), &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind() : get_symbol_offset\n");
        fprintf(stderr, "ERROR: %d\n", error);
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
        perror("error in drsym_lookup_symbol() : get_symbol_offset\n");
        fprintf(stderr, "ERROR: %d\n", error);
        return 0;
    } else {
        // printf("offset: %d\n", offset);
    }
    
    drsym_exit();

    return offset;
}


std::vector<std::string> get_all_symbols(const char * module_name, drsym_flags_t flag) {
    drsym_init(NULL);
    drsym_error_t error;
    drsym_debug_kind_t kind;
    
    error = drsym_get_module_debug_kind(module_name, &kind);
    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_get_module_debug_kind()\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw "error";
    } else {
        // printf("kind: %d\n", kind);
    }

    std::vector<std::string> syms;
    error = drsym_enumerate_symbols_ex(module_name,
                                    callback_get_all_symbols_1,
                                    sizeof(drsym_info_t),
                                    &syms,
                                    flag);
    /*error = drsym_enumerate_symbols(module_name,
                                    callback_get_all_symbols_2,
                                    &syms,
                                    flag);*/
    
    module_data_t * module = dr_lookup_module_by_name(module_name);
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\"", module_name);
    }
    // все импортируемые функции
    dr_symbol_import_iterator_t * sym_imp_iter = dr_symbol_import_iterator_start(module->handle, NULL);
    while (dr_symbol_import_iterator_hasnext(sym_imp_iter)) {
        dr_symbol_import_t * symbol = dr_symbol_import_iterator_next(sym_imp_iter);
        if (symbol != NULL) {
            syms.push_back(symbol->name);
        }
    }
    dr_symbol_import_iterator_stop(sym_imp_iter);
    dr_free_module_data(module);

    if (error != DRSYM_SUCCESS) {
        perror("error in drsym_lookup_symbol()\n");
        fprintf(stderr, "ERROR: %d\n", error);
        throw "error";
    } else {
        // printf("HIIII");
    }
    
    drsym_exit();

    return syms;
}


bool callback(const char * name, size_t modoffs, void * data) {
    // printf("symbol: %s\n", name);
    if (name && name[0] == '_') {
        if (strstr(name, "_asm_sort")) {
            // return false;
            printf("symbol: %s\n", name);
            printf("modoffs: %zu\n", modoffs);
        }
    }
    return true;
}

std::vector<std::string> get_functions_names(const char * module_name) {
    std::vector<std::string> func_names;
    if (module_name == NULL) {
        perror("NULL module name has been passed!\n");
        throw std::invalid_argument("NULL module name");
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
        throw std::invalid_argument("error in drsym_get_module_debug_kind()");
    } else {
        // printf("kind: %d\n", kind);
    }

    printf(
        "=========================================\n"
        "###########  export symbols  ############\n"
        "=========================================\n"
        );
    // все экспортируемые функции
    dr_symbol_export_iterator_t * sym_exp_iter = dr_symbol_export_iterator_start(module->handle);
    while (dr_symbol_export_iterator_hasnext(sym_exp_iter)) {
        dr_symbol_export_t * symbol = dr_symbol_export_iterator_next(sym_exp_iter);
        if (symbol != NULL) {
            if (strstr(symbol->name, "_asm_sort")) {
                printf("symbol iter: %p\n"
                        "\tname: %s\n"
                        "\taddr: %p\n",
                        sym_exp_iter, symbol->name, symbol->addr);
            }
            func_names.push_back(symbol->name);
        }
    }
    dr_symbol_export_iterator_stop(sym_exp_iter);

    printf(
        "=========================================\n"
        "###########  import symbols  ############\n"
        "=========================================\n"
        );
    // все экспортируемые функции
    dr_symbol_import_iterator_t * sym_imp_iter = dr_symbol_import_iterator_start(module->handle, NULL);
    while (dr_symbol_import_iterator_hasnext(sym_imp_iter)) {
        dr_symbol_import_t * symbol = dr_symbol_import_iterator_next(sym_imp_iter);
        if (symbol != NULL) {
            if (strstr(symbol->name, "_asm_sort")) {
                printf("symbol iter: %p\n"
                        "\tname: %s\n",
                        sym_imp_iter, symbol->name);
            }
            func_names.push_back(symbol->name);
        }
    }
    dr_symbol_import_iterator_stop(sym_imp_iter);

    printf(
        "=========================================\n"
        "##########  enumerate symbols  ##########\n"
        "=========================================\n"
        );
    char func_name[] = "_asm_sort";
    error = drsym_enumerate_symbols(module_name,
                                    callback,
                                    func_name,
                                    DRSYM_DEMANGLE_FULL);


    drsym_exit();
    dr_free_module_data(module);

    return func_names;
}

std::vector<sym_info_t> filter_symbols(const std::vector<sym_info_t> & symbols, const std::string module_name) {
    module_data_t * module = dr_lookup_module_by_name(module_name.c_str());
    if (module == NULL) {
        fprintf(stderr, "cannot load module with name \"%s\"", module_name.c_str());
    }

    drsym_init(NULL);
    std::vector<sym_info_t> filtered_symbols;
    char * buf = (char *) malloc(1000);
    drsym_type_t * expanded_type = NULL;
    drsym_error_t error;
    for (auto & sym : symbols) {
        // generic_func_t func = dr_get_proc_address(module->handle, sym.name.c_str());
        error = drsym_expand_type(  module_name.c_str(),
                            sym.type_id,
                            1,
                            buf,
                            1000,
                            &expanded_type);
        if (error != DRSYM_SUCCESS) {
            std::cout << "error: " << error << std::endl;
        }
        // if (expanded_type->kind == DRSYM_TYPE_PTR) {
        //     filtered_symbols.push_back(sym);
        // }
    }
    drsym_exit();

    dr_free_module_data(module);
    return filtered_symbols;
}

std::vector<sym_info_t> filter_from_list(const std::vector<sym_info_t> & symbols, const std::vector<std::string> & sym_list) {
    std::vector<sym_info_t> res;
    // можно оптимизировать через какое-нибудь RB-tree
    for (auto & sym : sym_list) {
        auto iter = std::find_if(symbols.begin(), symbols.end(), [&sym](const sym_info_t & el){return el.name.find(sym) != std::string::npos;});
        if (iter != symbols.end()) {
            res.push_back(*iter);
            printf("%s\n", iter->name.c_str());
        } else {
            printf("not found: %s!\n", sym.c_str());
        }
    }
    return res;
}


struct ModuleInfo {
    std::string name;
    std::string path;
};

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

void print_all_imported_symbols() {
    drsym_init(NULL);

    auto modules = get_all_modules();
    for (auto module_info : modules) {
        auto module_name = module_info.name;
        dr_printf("module_name: %s\n", module_name.c_str());
        auto module = dr_lookup_module_by_name(module_name.c_str());
        dr_get_proc_address(module->handle, "New_G1");

        auto iterator_im = dr_symbol_import_iterator_start(module->handle, NULL);
        do {
            auto * symbol = dr_symbol_import_iterator_next(iterator_im);
            dr_printf("symbol: %s\n", symbol->name);
        } while (dr_symbol_import_iterator_hasnext(iterator_im));
        dr_symbol_import_iterator_stop(iterator_im);
    }

    drsym_exit();
}

void print_modules() {
    auto modules = get_all_modules();
    dr_printf("modules:\n");
    for (auto module : modules) {
        dr_printf("\tmodule_name: %s; module_path: %s\n", module.name.c_str(), module.path.c_str());
    }
}

std::string int_to_hex(int my_integer) {
    std::stringstream sstream;
    sstream << std::hex << my_integer;
    std::string result = sstream.str();
    return result;
}

#endif // FUNCS_DR_header
