#ifndef MY_DEBUG_header
#define MY_DEBUG_header

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
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

void print_all_imported_symbols() {
    drsym_init(NULL);

    auto modules = get_all_modules();
    for (auto module_info : modules) {
        auto module_name = module_info.name;
        dr_printf("module_name: %s\n", module_name.c_str());
        auto module = dr_lookup_module_by_name(module_name.c_str());
        // dr_get_proc_address(module->handle, "New_G1");

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

void print_instruction(void *drcontext, instr_t *instr) {
    char instr_str[256];
    instr_disassemble_to_buffer(drcontext, instr, instr_str, sizeof(instr_str));
    dr_printf("Instruction: %s\n", instr_str);
}

void print_argv(int argc, const char *argv[]) {
    dr_printf("command line args:\n");
    for (int i=0; i < argc; ++i) {
        dr_printf("\t%d: %s\n", i, argv[i]);
    }
}

#endif // MY_DEBUG_header