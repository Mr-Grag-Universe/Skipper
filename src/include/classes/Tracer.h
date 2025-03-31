/**
 * @file Tracer.h
 * @author Stepan Kafanov
 * @brief Tracer class description
 * @version 0.1
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef MY_TRACER_header
#define MY_TRACER_header

#define X86

#include "Config.h"

/// @brief just pair-structure <code segment start offset, code segment end offset>
struct CodeSegmentDescriber {
    size_t start;
    size_t end;
};

/// @brief extra-counters section (memory area for extra trace for libfuzzer), contains bounds and size.
struct TraceArea {
    size_t start;
    size_t end;
    size_t size;
};

/// @brief gets most signifant bit of `x` digit
int get_msb_ind(uint x) {
    if (x == 0)
        return -1; // if there is not any rised bits (==0)

    int msb_index = 0;
    while (x >>= 1) {
        msb_index++;
    }
    return msb_index;
}

/**
 * @brief trace function for clean-call
 * @details
 * This function will be called before each instruction client is tracing.
 * It uses buffer of size 65 bytes to trace MSB (most significant bit) of tracing instruction operand and current carry flag from flag register.
 * 
 * @param offset_int_ptr - trace-memory address
 * @param size - trace-memory size
 * @param ind - tracing instruction's index
 * @param reg_id - register ID in DynamoRIO
 * @return 0 - error code
 */
int trace_overflow(int* offset_int_ptr, uint32_t size, uint32_t ind, uint32_t reg_id) {
    char* offset = (char*) offset_int_ptr;
    reg_id_t dst_reg = (reg_id_t) reg_id;
    if (size < 65*(ind+1)) {
        // TODO : cheking
        printf("memory is not enough for tracing\n");
    }

    // restore context
    dr_mcontext_t mc = { sizeof(mc), DR_MC_ALL};
    dr_get_mcontext(dr_get_current_drcontext(), &mc);

    // flag register
    reg_t xflags = mc.xflags;
    // target register
    reg_t reg = reg_get_value(dst_reg, &mc);

    // most signigicant bit
    int msb_ind_reg = get_msb_ind((uint) reg);

    // trace
    if (msb_ind_reg >= 0) {
        ((char *)offset)[(ind*65+msb_ind_reg) % size] += 1;
    }
    ((char *)offset)[(ind*65+64) % size] += xflags & EFLAGS_CF;
    
    return 0;
}

/**
 * @brief Function for inline ASM insertion
 * @details
 * Inserts ASM instraction directly before tracing ASM-instruction.
 * Functional is the same as in trace_overflow function.
 * 
 * @param drcontext - DR context
 * @param tag - DR tag
 * @param bb - current basic block
 * @param where - current ASM instruction
 * @param offset - trace-memory address
 * @param size - trace-memory size
 * @param ind - tracing instruction's index
 * @param reg_id - register ID in DynamoRIO
 */
void insert_tracing(void *drcontext, void *tag, instrlist_t *bb, instr_t *where, 
                    char* offset, uint32_t size, uint32_t ind, uint32_t reg_id) {
    auto xax = DR_REG_XAX;
    auto xbx = DR_REG_XBX;
    auto xcx = DR_REG_XCX;
    auto xdx = DR_REG_XDX;

    // save registers and flags
    dr_save_arith_flags(drcontext, bb, where, SPILL_SLOT_2); // store into xax by default
    dr_save_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_save_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_save_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_save_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    
    instr_t * instr;

    // save interesting reg value into RCX
    instr = XINST_CREATE_move(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(reg_id)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // =========================================================================
    // get_msb_ind
    // RCX - research register
    // RAX - return result
    // create labels mgb_loop, msb_finish, msb_ret_minus_1

    instr = INSTR_CREATE_xor(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RAX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_bsr(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RCX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        OPND_CREATE_INT64(0x3f)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_and(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RCX)
    );
    instrlist_meta_preinsert(bb, where, instr);


    // ==============================================================================
    
    instr = XINST_CREATE_move(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(DR_REG_RAX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // answer in RCX
    // write address into RAX
    auto pos = ind*65;
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        OPND_CREATE_INT64(offset+pos)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RCX)
    );
    instrlist_meta_preinsert(bb, where, instr);

    instr = INSTR_CREATE_mov_ld(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        OPND_CREATE_MEM8(DR_REG_RAX, 0)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // add 1
    instr = INSTR_CREATE_inc(
        drcontext,
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // store value
    instr = XINST_CREATE_store(
        drcontext,
        OPND_CREATE_MEM8(DR_REG_RAX, 0),
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // =========================================================================
    // occupied RAX for address, RCX for flags, RDX - supportive
    // read saved flags and store them to reg RCX
    dr_restore_reg(
        drcontext, bb, where, (reg_id_t) DR_REG_RCX, SPILL_SLOT_2
    );
    instr = INSTR_CREATE_and(
        drcontext,
        opnd_create_reg(DR_REG_ECX),
        OPND_CREATE_INT32((uint32_t) EFLAGS_CF)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // store flag
    auto i = (ind*65+64) % size;
    // form address
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        OPND_CREATE_INT64(offset)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // read byte
    instr = INSTR_CREATE_mov_ld(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        OPND_CREATE_MEM8(DR_REG_RAX, i)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // plus CF bit
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        opnd_create_reg(DR_REG_CL)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // store new value
    instr = XINST_CREATE_store(
        drcontext,
        OPND_CREATE_MEM8(DR_REG_RAX, i),
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // restore flags and registers
    dr_restore_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    dr_restore_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_restore_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_restore_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_restore_arith_flags(drcontext, bb, where, SPILL_SLOT_2);
}

/**
 * @brief Class, that inserts tracing function in basic blocks.
 * 
 */
class Tracer {
    TraceArea trace_area;
    /// @brief hash-map of ASM instructions under trace and their indexes in Trace class object
    std::map<app_pc, size_t> pc_ind_map;
    /// @brief Registers ID hash-map
    std::map<reg_id_t, size_t> reg_ind_map;
public:
    json tracer_config;
    Tracer() {}

    /// @brief set up configuration for client tracing
    bool set_config(Configurator config) {
        std::cout << "setting config to tracer!" << std::endl;
        this->tracer_config = config.getTracerConfig();
        return true;
    }

    /// @brief set up trace-memory (extra-counters section) address
    void set_trace_area(size_t start, size_t end) {
        this->trace_area = {start, end, end-start};
    }

protected:
    /// @brief Get register ID if submited and submit if it is not.
    int get_reg_id(reg_id_t reg) {
        auto iter = this->reg_ind_map.find(reg);
        if (iter != this->reg_ind_map.end()) {
            return this->reg_ind_map[reg];
        }
        this->reg_ind_map[reg] = this->reg_ind_map.size();
        return this->reg_ind_map[reg];
    }
public:
    /**
     * @brief This method inserts instrumentation into basic-blocks
     * @details `use_asm` parameter regulates whether client shall use inline ASM insertion or clean call DynamoRIO function to insert tracing.
     * 
     * @param drcontext - DR context
     * @param tag - DR tag
     * @param bb - current basic block
     * @param instr - current ASM instruction
     */
    void traceOverflow(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr) {
        if (!instr_num_dsts(instr)) {
            return;
        }

        opnd_t dst = instr_get_dst(instr, 0);
        if (!opnd_is_reg(dst)) {
            return;
        }

        reg_id_t dst_reg = opnd_get_reg(dst);
        int reg_ind = this->get_reg_id(dst_reg);

        app_pc instr_pc = instr_get_app_pc(instr);
        // new instruction? - set index
        if (this->pc_ind_map.find(instr_pc) == this->pc_ind_map.end()) {
            this->pc_ind_map[instr_pc] = this->pc_ind_map.size();
        }
        size_t ind = this->pc_ind_map[instr_pc];

        auto * module = dr_get_main_module();
        auto pc = module->start;
        dr_free_module_data(module);
        size_t start_size_t = (size_t) this->trace_area.start + (size_t) pc;

        instr_t *nxt = instr_get_next(instr);
        if (this->tracer_config["use_asm"]) {
            if (this->tracer_config["debug"]) {
                main_logger.log_debug("using inline-asm instrumentation!");
                dr_printf("[DEBUG] : using inline-asm instrumentation!\n");
            }

            insert_tracing(drcontext, tag, bb, nxt, 
                          (char*) start_size_t, this->trace_area.size, ind, dst_reg);
        } else {
            if (this->tracer_config["debug"]) {
                main_logger.log_debug("using clean_call instrumentation!");
                dr_printf("[DEBUG] : using clean_call instrumentation!\n");
            }
            
            dr_insert_clean_call_ex(drcontext, 
                                    bb, nxt, 
                                    (void *) trace_overflow, 
                                    (dr_cleancall_save_t) (DR_CLEANCALL_READS_APP_CONTEXT | DR_CLEANCALL_MULTIPATH),
                                    4, 
                                    OPND_CREATE_INTPTR(start_size_t),
                                    OPND_CREATE_INT32(this->trace_area.size),
                                    OPND_CREATE_INT32(ind),
                                    OPND_CREATE_INT32(dst_reg));
        }
        if (this->tracer_config["debug"]) {
            main_logger.log_debug("add number: {} | add index: {} | thread id: {}", 
                                  this->pc_ind_map.size(), ind, get_thread_id());
            dr_printf("[DEBUG] : add number: %d | add index: %d | thread id: %s\n", 
                      this->pc_ind_map.size(), ind, get_thread_id().c_str());
        }
    }
};

#endif // MY_TRACER_header
