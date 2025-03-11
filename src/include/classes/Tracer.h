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

int trace_overflow(int* offset_int_ptr, uint32_t size, uint32_t ind, uint32_t reg_id) {
    // offset - адрес памяти, куда писать
    // size - размер памяти
    // ind - индекс add
    // reg_id - индекс регистра в DynamoRIO

    char* offset = (char*) offset_int_ptr;
    reg_id_t dst_reg = (reg_id_t) reg_id;
    if (size < 65*(ind+1)) {
        // TODO : надо проверить, как работает
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

    // трейсим
    if (msb_ind_reg >= 0) {
        ((char *)offset)[(ind*65+msb_ind_reg) % size] += 1;
    }
    ((char *)offset)[(ind*65+64) % size] += xflags & EFLAGS_CF;
    
    return 0;
}

void insert_tracing(void *drcontext, void *tag, instrlist_t *bb, instr_t *where, 
                    char* offset, uint32_t size, uint32_t ind, uint32_t reg_id) {
    auto xax = DR_REG_XAX;
    auto xbx = DR_REG_XBX;
    auto xcx = DR_REG_XCX;
    auto xdx = DR_REG_XDX;

    // сохраняем регистры и флаги
    dr_save_arith_flags(drcontext, bb, where, SPILL_SLOT_2); // по умолчанию кладёт в xax
    dr_save_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_save_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_save_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_save_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    
    instr_t * instr;

    // сохраняем значение исследуемого регистра в RCX
    instr = XINST_CREATE_move(
        drcontext,
        opnd_create_reg(DR_REG_RCX),
        opnd_create_reg(reg_id)
    );
    instrlist_meta_preinsert(bb, where, instr);

    // =========================================================================
    // get_msb_ind
    // RCX - исследуемый регистр хранится
    // RAX - возвращаемый результат
    // создаём метки mgb_loop, msb_finish, msb_ret_minus_1

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

    // возвращаем регистры и флаги на место
    dr_restore_reg(drcontext, bb, where, xdx, SPILL_SLOT_6);
    dr_restore_reg(drcontext, bb, where, xcx, SPILL_SLOT_5);
    dr_restore_reg(drcontext, bb, where, xbx, SPILL_SLOT_4);
    dr_restore_reg(drcontext, bb, where, xax, SPILL_SLOT_3);
    dr_restore_arith_flags(drcontext, bb, where, SPILL_SLOT_2);
}

class Tracer {
    TraceArea trace_area;
    std::map<app_pc, size_t> pc_ind_map;
    std::map<reg_id_t, size_t> reg_ind_map;
public:
    json tracer_config;
    Tracer() {}

    bool set_config(Configurator config) {
        std::cout << "setting config to tracer!" << std::endl;
        this->tracer_config = config.getTracerConfig();
        return true;
    }

    void set_trace_area(size_t start, size_t end) {
        this->trace_area = {start, end, end-start};
    }

protected:
    int get_reg_id(reg_id_t reg) {
        auto iter = this->reg_ind_map.find(reg);
        if (iter != this->reg_ind_map.end()) {
            return this->reg_ind_map[reg];
        }
        this->reg_ind_map[reg] = this->reg_ind_map.size();
        return this->reg_ind_map[reg];
    }
public:
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
        // если эту инструкцию ещё не встречали - выдаём ей номер
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
