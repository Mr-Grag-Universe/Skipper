#ifndef MY_GUARDER_H
#define MY_GUARDER_H

#include <vector>
#include <map>
#include <string>

#define X86
#include "dr_api.h"
#include "dr_tools.h"
#include "dr_ir_instr.h"

#include "funcs.h"

class Guarder {
public:
    bool good_lea_met = false;
    bool guards_opened = false;
    std::map<std::string, std::vector <long long int>> global_guards;

    Guarder() {}

    void set_global_guards(std::map<std::string, std::vector <long long int>> guards) {
        this->global_guards = guards;
    }

    bool throw_instr(instr_t * instr) {
        app_pc addr = instr_get_app_pc(instr);
        module_data_t *mod = dr_lookup_module(addr);
        auto module_name = std::string(dr_module_preferred_name(mod));
        dr_free_module_data(mod);

        int opcode = instr_get_opcode(instr);
        if (opcode == (int) OP_lea) {
            opnd_t src = instr_get_src(instr, 0);
            auto mem_addr = opnd_get_addr(src);
            if (std::find(  this->global_guards[module_name].begin(), 
                            this->global_guards[module_name].end(), 
                            (long long) mem_addr) != this->global_guards[module_name].end()) {
                this->good_lea_met = true;
            }
        } else if (this->good_lea_met && instr_writes_memory(instr)) {
            opnd_t src = instr_get_src(instr, 0);
            if (opnd_is_immed_int(src)) {
                int val = opnd_get_immed_int(src);
                dr_printf("move opnd value is <%d>\n", val);
                this->guards_opened = (val == 1);

                if (this->guards_opened)
                    dr_printf("open the gates!\n");
                else
                    dr_printf("close the gates!\n");
            }
            this->good_lea_met = false;
        } else {
            this->good_lea_met = false;
        }
        return this->guards_opened;
    }
};

#endif // MY_GUARDER_H