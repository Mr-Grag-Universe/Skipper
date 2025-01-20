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

void get_msb_ind_cc(uint x, int * res) {
    if (x == 0)
        return; // Возвращает -1, если нет установленных битов

    int msb_index = 0;
    while (x >>= 1) {
        msb_index++;
    }
    // return msb_index;
    *res = msb_index;
}

int trace_overflow(int* offset_int_ptr, uint32_t size, uint32_t ind, uint32_t reg_id) {
    // offset - адрес памяти, куда писать
    // size - размер памяти
    // ind - индекс add
    // reg_id - индекс регистра в DynamoRIO

    char* offset = (char*) offset_int_ptr; // (char*) ((((size_t) high) << 32) | (size_t) low);
    reg_id_t dst_reg = (reg_id_t) reg_id;
    if (size < 65*(ind+1)) {
        printf("memory is not enough for tracing\n");
    }

    // восстанавливаем контекст
    dr_mcontext_t mc = { sizeof(mc), DR_MC_ALL};
    dr_get_mcontext(dr_get_current_drcontext(), &mc);

    // регистр флагов
    reg_t xflags = mc.xflags;
    // регистр назначения
    reg_t reg = reg_get_value(dst_reg, &mc);

    // находим индекс старшего бита
    int msb_ind_reg = get_msb_ind((uint) reg);

    // return 0;
    // трейсим
    if (msb_ind_reg >= 0) {
        // printf("address: %p + %d\n", offset, ind*65+msb_ind_reg);
        ((char *)offset)[(ind*65+msb_ind_reg) % size] += 1;
    }
    ((char *)offset)[(ind*65+64) % size] += xflags & EFLAGS_CF;
    
    return 0;
}

void instrument(void *drcontext, void *tag, instrlist_t *bb, instr_t *where, 
                char* offset, uint32_t size, uint32_t ind, uint32_t reg_id) {
    auto xax = DR_REG_XAX;
    auto xbx = DR_REG_XBX;
    auto xcx = DR_REG_XCX;
    auto xdx = DR_REG_XDX;
    auto xdi = DR_REG_XDI;
    auto xsi = DR_REG_XSI;

    // сохраняем регистры и флаги
    dr_save_arith_flags(drcontext, bb, where, SPILL_SLOT_2); // по умолчанию кладёт в xax
    dr_save_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_save_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_save_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_save_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    dr_save_reg(drcontext, bb, where, xdi, SPILL_SLOT_7);
    dr_save_reg(drcontext, bb, where, xsi, SPILL_SLOT_8);
    
    
    instr_t * instr;

    // сохраняем значение исследуемого регистра в RCX
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(reg_id)
    );

    // ====================================================================
    // проверка на то, что памяти хватит
    /*
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_EDX),
        OPND_CREATE_INT32(1)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = XINST_CREATE_move(
        drcontext,
        opnd_create_reg(DR_REG_EDX),
        opnd_create_reg(DR_REG_EBX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_shl(
        drcontext,
        opnd_create_reg(DR_REG_EBX),
        opnd_create_immed_int(6, OPSZ_1)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_EBX),
        opnd_create_reg(DR_REG_EDX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    */
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_EBX),
        OPND_CREATE_INT32(65*(ind+1))
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_ESI),
        OPND_CREATE_INT32(size)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_cmp(
        drcontext,
        opnd_create_reg(DR_REG_EBX),
        opnd_create_reg(DR_REG_ESI)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr_t * finish = INSTR_CREATE_label(drcontext);
    opnd_t opnd1 = opnd_create_instr(finish);
    instr = INSTR_CREATE_jcc(
        drcontext,
        OP_jb,
        opnd1
    );
    // instrlist_meta_preinsert(bb, where, instr);
    // checked

    // =========================================================================
    // get_msb_ind
    // RCX - исследуемый регистр хранится
    // RAX - возвращаемый результат
    // создаём метки mgb_loop, msb_finish, msb_ret_minus_1
    instr_t * msb_finish      = INSTR_CREATE_label(drcontext);
    instr_t * msb_loop        = INSTR_CREATE_label(drcontext);
    instr_t * msb_ret_minus_1 = INSTR_CREATE_label(drcontext);
    // if (x == 0) return -1; 
    instr = INSTR_CREATE_test(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(DR_REG_RCX)
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_jcc(
        drcontext,
        OP_je,
        opnd_create_instr(msb_ret_minus_1) // get_msb_ind continue
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_xor(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RAX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_shr(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_immed_int(1, OPSZ_1)
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_jcc(
        drcontext,
        OP_je,
        opnd_create_instr(msb_finish) // get_msb_ind continue
    );
    // instrlist_meta_preinsert(bb, where, instr);
    // instrlist_meta_preinsert(bb, where, msb_loop);
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_EAX),
        OPND_CREATE_INT32(1)
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_shr(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_immed_int(1, OPSZ_1)
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_jcc(
        drcontext,
        OP_jne,
        opnd_create_instr(msb_loop) // loop
    );
    // instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_jmp(
        drcontext,
        opnd_create_instr(msb_finish) // msb finish
    );
    // instrlist_meta_preinsert(bb, where, instr);
    // instrlist_meta_preinsert(bb, where, msb_ret_minus_1);
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        OPND_CREATE_INT64(-1)
    );
    // instrlist_meta_preinsert(bb, where, instr);
    // instrlist_meta_preinsert(bb, where, msb_finish);
    
    instr = INSTR_CREATE_bsr(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RCX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        OPND_CREATE_INT64(128-1)
    );
    instrlist_meta_preinsert(bb, where, instr);
    instr = INSTR_CREATE_and(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        opnd_create_reg(DR_REG_RCX)
    );
    instrlist_meta_preinsert(bb, where, instr);


    // ==============================================================================
    // теперь результат функции помещаем в RCX и инкрементаируем соответствующий байт
    
    instr = XINST_CREATE_move(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(DR_REG_RAX)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // ответ в RCX
    // записываем адрес в RAX
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
    
    // делать %size не обязательно, т.к. ранее я произвёл проверку
    // instr = INSTR_CREATE_div_4(
    //     drcontext,
    //     OPND_CREATE_
    // );
    // instrlist_meta_preinsert(bb, where, instr);
    // читаем, что лежит по адресу данного байта

    instr = INSTR_CREATE_mov_ld(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        OPND_CREATE_MEM8(DR_REG_RAX, 0)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // прибавляем 1
    instr = INSTR_CREATE_inc(
        drcontext,
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // кладём новое значение по адресу
    instr = XINST_CREATE_store(
        drcontext,
        OPND_CREATE_MEM8(DR_REG_RAX, 0),
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // =========================================================================
    // заняты RAX под адрес, RCX под флаги, RDX - вспомогательный
    // берём сохранённые флаги и кладём в reg RCX
    dr_restore_reg(
        drcontext, bb, where, (reg_id_t) DR_REG_RCX, SPILL_SLOT_2
    );
    // по идее их всего 6 и должно прокатить, если я использую eax вместо rax
    instr = INSTR_CREATE_and(
        drcontext,
        opnd_create_reg(DR_REG_ECX),
        OPND_CREATE_INT32((uint32_t) EFLAGS_CF)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // сохраняем в памяти флаг
    auto i = (ind*65+64) % size;
    // формируем адрес
    instr = INSTR_CREATE_mov_imm(
        drcontext,
        opnd_create_reg(DR_REG_RAX),
        OPND_CREATE_INT64(offset)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // читаем, что лежит по адресу данного байта
    instr = INSTR_CREATE_mov_ld(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        OPND_CREATE_MEM8(DR_REG_RAX, i)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // прибавляем бит CF регистра флагов
    instr = INSTR_CREATE_add(
        drcontext,
        opnd_create_reg(DR_REG_DL),
        opnd_create_reg(DR_REG_CL)
    );
    instrlist_meta_preinsert(bb, where, instr);
    // кладём новое значение по адресу
    instr = XINST_CREATE_store(
        drcontext,
        OPND_CREATE_MEM8(DR_REG_RAX, i),
        opnd_create_reg(DR_REG_DL)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // конец вставки
    instrlist_meta_preinsert(bb, where, finish);

    // возвращаем регистры и флаги на место
    dr_restore_reg(drcontext, bb, where, xsi, SPILL_SLOT_8);
    dr_restore_reg(drcontext, bb, where, xdi, SPILL_SLOT_7);
    dr_restore_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    dr_restore_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_restore_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_restore_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_restore_arith_flags(drcontext, bb, where, SPILL_SLOT_2);
}

class Tracer {
    std::vector <reg_id_t> work_registers;
    reg_id_t flag_register;
    TraceArea trace_area;
    std::map<app_pc, size_t> pc_ind_map;
    std::map<reg_id_t, size_t> reg_ind_map;
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
    std::vector<instr_t *> construct_asmtrace_overflow( void *   drcontext, 
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

    int get_reg_id(reg_id_t reg) {
        auto iter = this->reg_ind_map.find(reg);
        if (iter != this->reg_ind_map.end()) {
            return this->reg_ind_map[reg];
        }
        this->reg_ind_map[reg] = this->reg_ind_map.size();
        return this->reg_ind_map[reg];
    }

public:
    void traceInstruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr) {
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

    void traceOverflow(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr) {
        // if (!instr_writes_memory(instr)) {
        if (!instr_num_dsts(instr)) {
            dr_printf("<1> cannot instrument this!\n");
            return;
        }

        opnd_t dst = instr_get_dst(instr, 0);
        if (!opnd_is_reg(dst)) {
            dr_printf("<2> cannot instrument this!\n");
            return;
        }

        reg_id_t dst_reg = opnd_get_reg(dst);
        int reg_ind = this->get_reg_id(dst_reg);

        app_pc instr_pc = instr_get_app_pc(instr);
        // если эту инструкцию ещё не встречали - выдаём ей номер
        if (this->pc_ind_map.find(instr_pc) == this->pc_ind_map.end()) {
            this->pc_ind_map[instr_pc] = this->pc_ind_map.size();
        }
        size_t ind = this->pc_ind_map[instr_pc];

        auto * module = dr_get_main_module();
        auto pc = module->start;
        dr_free_module_data(module);
        size_t start_size_t = (size_t) this->trace_area.start + (size_t) pc;

        uint32_t high = (uint32_t)(start_size_t >> 32); // Старшая часть
        uint32_t low = (uint32_t)(start_size_t & 0xFFFFFFFF); // Младшая часть

        // return;
        instr_t *nxt = instr_get_next(instr);
        std::cout << this->tracer_config["use_asm"] << std::endl;
        if (this->tracer_config["use_asm"]) {
            dr_printf("using inline-asm instrumentation!\n");
            instrument(drcontext, tag, bb, nxt, 
                       (char*) start_size_t, this->trace_area.size, ind, dst_reg);
        } else {
            dr_printf("using clean_call instrumentation!\n");
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
        
        // проверка
        dr_printf("add number: %d | add index: %d | thread id: %s\n", 
                    this->pc_ind_map.size(), ind, get_thread_id().c_str());
    }
};

#endif // MY_TRACER_header
