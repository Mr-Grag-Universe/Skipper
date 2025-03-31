/**
 * @file Guarder.h
 * @author Stepan Kafanov
 * @brief Class for LLVM-guards tracking
 * @version 0.1
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef MY_GUARDER_H
#define MY_GUARDER_H

#include <vector>
#include <map>
#include <string>

#define X86
#include "dr_api.h"
#include "dr_tools.h"
#include "dr_ir_instr.h"

#include "../funcs.h"

/**
 * @brief This class is responsible for tracking the opening and closing of the "gates"
 * 
 */
class Guarder {
protected:
    /// Tells wether we met lea instr with expected operands
    bool good_lea_met_ = false;
    /// Tells wether "guards" were opened or not
    bool guards_opened_ = false;
    std::map<std::string, std::vector <long long int>> global_guards_;

public:
    Guarder() {}

    /**
     * @brief Set the global guards object
     * 
     * @param guards 
     */
    void set_global_guards(std::map<std::string, std::vector <long long int>> guards) {
        this->global_guards_ = guards;
    }

    /**
     * @brief wether LLVM "guards" are opened
     * 
     * @return 
     */
    bool guards_opened() const {
        return this->guards_opened_;
    }

    /**
     * @brief Update LLVM-guards state, using current instruction
     * @details Use fixed pattern:
     * + LEA guard-address
     * + MOV 
     * 
     * If mov 1 - guards are opening here, else guards are closing
     * 
     * @param instr - current basic block instruction
     * @return wheter or not LLVM-gurds are opened now
     */
    bool throw_instr(instr_t * instr) {
        app_pc addr = instr_get_app_pc(instr);
        module_data_t *mod = dr_lookup_module(addr);
        auto module_name = std::string(dr_module_preferred_name(mod));
        dr_free_module_data(mod);

        int opcode = instr_get_opcode(instr);
        if (instr_num_srcs(instr)) {
            opnd_t src = instr_get_src(instr, 0);
            if (opcode == (int) OP_lea && (opnd_is_far_memory_reference(src) || opnd_is_near_memory_reference(src)) && opnd_is_abs_addr(src)) {
                dr_printf("2 opnd get addr\n");
                auto mem_addr = opnd_get_addr(src);
                dr_printf("addr has been gotten\n");

                if (std::find(  this->global_guards_[module_name].begin(), 
                                this->global_guards_[module_name].end(), 
                                (long long) mem_addr) != this->global_guards_[module_name].end()) {
                    this->good_lea_met_ = true;
                } else {
                    this->good_lea_met_ = false;
                }
            } else {
                if (this->good_lea_met_ && instr_writes_memory(instr)) { 
                    if (opnd_is_immed_int64(src)) {
                        long val = opnd_get_immed_int64(src);
                        dr_printf("move opnd value is <%ld>\n", val);
                        this->guards_opened_ = (val == 1);

                        if (this->guards_opened_)
                            dr_printf("open the gates!\n");
                        else
                            dr_printf("close the gates!\n");
                    }
                }
                this->good_lea_met_ = false;
            }
        } else {
            this->good_lea_met_ = false;
        }
        return this->guards_opened_;
    }
};

#endif // MY_GUARDER_H