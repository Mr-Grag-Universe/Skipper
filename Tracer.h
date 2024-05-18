#ifndef MY_TRACER_header
#define MY_TRACER_header

#define X86

#include "Config.h"

struct CodeSegmentDescriber {
    size_t start;
    size_t end;
};

struct TraceArea {
    size_t start;
    size_t end;
    size_t size;
};

int get_msb_ind(uint x) {
    if (x == 0)
        return -1; // Возвращает -1, если нет установленных битов

    int msb_index = 0;
    while (x >>= 1) {
        msb_index++;
    }
    return msb_index;
}

int trace_add(uint32_t offset, uint32_t size, uint32_t ind) {
    // offset - адрес памяти, куда писать
    // size - размер памяти
    // ind - индекс add
    size_t n = 5;
    if (size < 64*n*(ind+1)) {
        printf("memory is not enough for tracing\n");
    }

    // восстанавливаем контекст
    dr_mcontext_t mc;
    dr_get_mcontext(dr_get_current_drcontext(), &mc);

    reg_t xflags = mc.xflags;
    reg_t xax = reg_get_value(DR_REG_XAX, &mc);
    reg_t xbx = reg_get_value(DR_REG_XBX, &mc);
    reg_t xcx = reg_get_value(DR_REG_XCX, &mc);
    reg_t xdx = reg_get_value(DR_REG_XDX, &mc);

    reg_t regs[] = {xflags, xax, xbx, xcx, xdx};
    int inds[n];
    
    for (size_t i = 0; i < n; ++i) {
        inds[i] = get_msb_ind((uint) regs[i]);
    }

    for (size_t i = 0; i < n; ++i) {
        if (inds[i] >= 0) {
            dr_printf("XXXXXXX %d\n", inds[i]);
            ((char *)offset)[(ind*64*n+i*64+inds[i]) % size]++;
        }
    }

    return 0;
}

class Tracer {
    std::vector <reg_id_t> work_registers;
    reg_id_t flag_register;
    TraceArea trace_area;
    std::map<app_pc, size_t> pc_ind_map;
public:
    json tracer_config;
    Tracer(const Configurator & config) {
        this->tracer_config = config.getTracerConfig();
    }

    void set_registers(reg_id_t flag_register, std::vector <reg_id_t> work_registers)
    {
        /*
         * должно быть только так: eax, {ebx, ecx, edx, ...}
         */
        this->work_registers = work_registers;
        this->flag_register = flag_register;

        if (std::find(work_registers.begin(), work_registers.end(), flag_register) == work_registers.end()) {
            fprintf(stderr, "There is not flag_register among work registers. I added it there for you =)\n");
            this->work_registers.push_back(flag_register);
        }
    }
    void set_trace_area(size_t start, size_t end)
    {
        this->trace_area = {start, end, end-start};
    }

protected:
    std::vector<instr_t *> construct_asmtrace_add_overflow( void *   drcontext, 
                                                            size_t current_location,
                                                            size_t trace_address) 
    {
        /*
         * предполагается, что мы сохранили n регистров к SLOT 1...n и теперь можем их dr_read_saved_reg()
         * 
         * поскольку номер SLOT надо вбивать вручную, сделать адекватный цикл не получится
         * 
         * reg_t reg = dr_read_saved_reg()
         * offset = n*64*add_ind+i
         * bit_ind = 
        */
        reg_id_t    xax = this->flag_register, 
                    xbx = this->work_registers[0], 
                    xcx = this->work_registers[1], 
                    xdx = this->work_registers[2];


        instr_t * instr_1 = XINST_CREATE_move(      drcontext, 
                                                    opnd_create_reg(xcx),
                                                    opnd_create_reg(xax));

        // reg_t == uint
        reg_t reg = dr_read_saved_reg(drcontext, SPILL_SLOT_1);



        std::vector<instr_t*> instrlist;
        instrlist.push_back(instr_1);

        return instrlist;
    }

    std::vector<instr_t *> construct_asm_code(  void *   drcontext, 
                                                size_t current_location,
                                                size_t trace_address)
    {
        /*
        * потом можно докрутить до красивого и рационального использования данных в 
        * распоряжение регистров
        */
        reg_id_t    xax = this->flag_register, 
                    xbx = this->work_registers[0], 
                    xcx = this->work_registers[1], 
                    xdx = this->work_registers[2];

        instr_t * instr_1 = XINST_CREATE_move(      drcontext, 
                                                    opnd_create_reg(xcx),
                                                    opnd_create_reg(xax));
        instr_t * instr_2 = INSTR_CREATE_mov_imm(   drcontext,
                                                    opnd_create_reg(xax), 
                                                    OPND_CREATE_INT32(current_location));
        instr_t * instr_3 = INSTR_CREATE_mov_imm(   drcontext,
                                                    opnd_create_reg(xbx), 
                                                    OPND_CREATE_INT32(this->trace_area.size-8));
        instr_t * instr_4 = INSTR_CREATE_xor(       drcontext,
                                                    opnd_create_reg(xdx),
                                                    opnd_create_reg(xdx));
        instr_t * instr_5 = INSTR_CREATE_div_4(     drcontext,
                                                    opnd_create_reg(xbx));
        // теперь в xdx у нас отступ в массиве extra_counter
        //        в xcx флаги
        instr_t * instr_6 = INSTR_CREATE_mov_imm(   drcontext,
                                                    opnd_create_reg(xax), 
                                                    OPND_CREATE_INT32(trace_address));
        instr_t * instr_7 = INSTR_CREATE_mov_ld(    drcontext,
                                                    opnd_create_reg(xbx),
                                                    OPND_CREATE_MEM32(xax, xdx));
        instr_t * instr_8 = INSTR_CREATE_add(       drcontext,
                                                    opnd_create_reg(xcx),
                                                    opnd_create_reg(xbx));
        instr_t * instr_9 = XINST_CREATE_store(     drcontext,
                                                    OPND_CREATE_MEM32(xax, xdx),
                                                    opnd_create_reg(xcx));

        std::vector<instr_t*> instrlist;
        instrlist.push_back(instr_1);
        instrlist.push_back(instr_2);
        instrlist.push_back(instr_3);
        instrlist.push_back(instr_4);
        instrlist.push_back(instr_5);
        instrlist.push_back(instr_6);
        instrlist.push_back(instr_7);
        instrlist.push_back(instr_8);
        instrlist.push_back(instr_9);

        return instrlist;
    }

    void insert_instructions(void * drcontext, instrlist_t * bb, instr_t * where, std::vector<instr_t *> new_instructions) 
    {
        for (auto & instr : new_instructions) {
            instrlist_meta_preinsert(bb, where, instr);
        }
    }

public:
    void trace_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr) {
        reg_id_t xcx = DR_REG_XCX, xdx = DR_REG_XDX, xax = DR_REG_XAX, xbx = DR_REG_XBX;

        // сохраняем регистры и флаги
        dr_save_arith_flags(drcontext, bb, instr, SPILL_SLOT_1); // по умолчанию кладёт в xax
        dr_save_reg(drcontext, bb, instr, xcx, SPILL_SLOT_2);
        dr_save_reg(drcontext, bb, instr, xdx, SPILL_SLOT_3);
        dr_save_reg(drcontext, bb, instr, xax, SPILL_SLOT_4);
        dr_save_reg(drcontext, bb, instr, xbx, SPILL_SLOT_5);

        app_pc pc = instr_get_app_pc(instr);
        uint32_t current_location = ((uint32_t)(uintptr_t)pc * (uint32_t)33533) & 0xFFFF;
        // dr_printf("curr_loc: %x\n", current_location);
        
        auto * module = dr_get_main_module();
        pc = module->start;
        dr_free_module_data(module);
        size_t start_size_t = (size_t) this->trace_area.start + (size_t) pc;

        auto new_bb = this->construct_asm_code(drcontext, current_location, start_size_t);
        this->insert_instructions(drcontext, bb, instr, new_bb);

        // возвращаем регистры и флаги на место
        dr_restore_reg(drcontext, bb, instr, xbx, SPILL_SLOT_5);
        dr_restore_reg(drcontext, bb, instr, xax, SPILL_SLOT_4);
        dr_restore_reg(drcontext, bb, instr, xdx, SPILL_SLOT_3);
        dr_restore_reg(drcontext, bb, instr, xcx, SPILL_SLOT_2);
        dr_restore_arith_flags(drcontext, bb, instr, SPILL_SLOT_1);
    }

    void trace_add_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr) {
        reg_id_t xax = DR_REG_XAX, xbx = DR_REG_XBX, xcx = DR_REG_XCX, xdx = DR_REG_XDX;

        app_pc instr_pc = instr_get_app_pc(instr);
        size_t ind = pc_ind_map.size();
        pc_ind_map[instr_pc] = ind;

        // сохраняем регистры и флаги
        dr_save_reg(drcontext, bb, instr, xax, SPILL_SLOT_1);
        dr_save_arith_flags(drcontext, bb, instr, SPILL_SLOT_5); // по умолчанию кладёт в xax
        dr_save_reg(drcontext, bb, instr, xbx, SPILL_SLOT_2);
        dr_save_reg(drcontext, bb, instr, xcx, SPILL_SLOT_3);
        dr_save_reg(drcontext, bb, instr, xdx, SPILL_SLOT_4);


        auto * module = dr_get_main_module();
        auto pc = module->start;
        dr_free_module_data(module);
        size_t start_size_t = (size_t) this->trace_area.start + (size_t) pc;

        instr_t *nxt = instr_get_next(instr);
        dr_insert_clean_call_ex(drcontext, 
                                bb, nxt, 
                                (void *) trace_add, 
                                DR_CLEANCALL_READS_APP_CONTEXT,
                                3, 
                                OPND_CREATE_INT32(start_size_t),
                                OPND_CREATE_INT32(this->trace_area.size),
                                OPND_CREATE_INT32(ind));

        dr_restore_reg(drcontext, bb, instr, xdx, SPILL_SLOT_4);
        dr_restore_reg(drcontext, bb, instr, xcx, SPILL_SLOT_3);
        dr_restore_reg(drcontext, bb, instr, xbx, SPILL_SLOT_2);
        dr_restore_arith_flags(drcontext, bb, instr, SPILL_SLOT_5); // по умолчанию кладёт в xax
        dr_restore_reg(drcontext, bb, instr, xax, SPILL_SLOT_1);
    }
};

#endif // MY_TRACER_header
