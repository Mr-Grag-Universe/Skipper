#include <algorithm>
#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <libelf.h>
#include <gelf.h>
#include <string>
#include <iostream>
#include <strstream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <vector>

#define X86

#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drsyms.h"
#include "dr_events.h"
#include "drx.h"
#include "dr_modules.h"
#include "dr_ir_opnd.h"
#include "dr_ir_macros_x86.h"
#include "dr_ir_opcodes_x86.h"

#include "include/funcs.h"
#include "include/find_pattern.h"
#include "include/func_bounds.h"
#include "include/thread_data.h"

static size_t start, stop;
static size_t start_local, stop_local;
void * lock;
static int tls_log_ind;
static int tls_data_ind;
static int tls_tracer_ind;
static std::pair<generic_func_t, generic_func_t> func_bounds;

static void callee(int32_t opcode, int* address, int*start, int*stop)
{
    // восстанавливаем контекст
    dr_mcontext_t mc;
    dr_get_mcontext(dr_get_current_drcontext(), &mc);

    printf("start-stop: %p-%p\n", start, stop);

    auto * module = dr_get_main_module();
    app_pc pc = module->start;
    dr_free_module_data(module);
    
    size_t start_size_t = (size_t) start + (size_t) pc;
    size_t stop_size_t = (size_t) stop + (size_t) pc;

    // проверяем нащупанную область памяти
    printf("some symbols from start: [%x|%x|%x|%x|%x]\n",   ((char*)start_size_t)[0], 
                                                            ((char*)start_size_t)[1], 
                                                            ((char*)start_size_t)[2], 
                                                            ((char*)start_size_t)[3], 
                                                            ((char*)start_size_t)[4]);
    
    // вычленяем биты и xor-им их с регистром флагов
    // uint flags = instr_get_arith_flags(address, DR_QUERY_INCLUDE_ALL);
}

static dr_emit_flags_t
bb_instrumentation_event_handler_old(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, bool for_trace, bool translating, void *user_data)
{
    FILE *log = (FILE *)(ptr_uint_t)drmgr_get_tls_field(drcontext, tls_log_ind);

    app_pc bb_addr_1 = dr_fragment_app_pc(tag);
    app_pc bb_addr_2 = dr_app_pc_for_decoding(bb_addr_1);

    if ((size_t)func_bounds.first <= (size_t)bb_addr_2 && (size_t)bb_addr_2 <= (size_t)func_bounds.second)
    {
        reg_id_t tls_data_reg = DR_REG_XCX, tls_loc_rdx = DR_REG_XDX, tls_loc_rax = DR_REG_XAX, ebx = DR_REG_XBX;

        // сохраняем регистры и флаги
        dr_save_arith_flags(drcontext, bb, instr, SPILL_SLOT_1);
        dr_save_reg(drcontext, bb, instr, tls_data_reg, SPILL_SLOT_2);
        dr_save_reg(drcontext, bb, instr, tls_loc_rdx, SPILL_SLOT_3);
        dr_save_reg(drcontext, bb, instr, tls_loc_rax, SPILL_SLOT_4);
        dr_save_reg(drcontext, bb, instr, ebx, SPILL_SLOT_5);

        // достаём адрес нашей структуры с данными потока
        drmgr_insert_read_tls_field(drcontext, tls_data_ind, bb, instr, tls_data_reg);

        app_pc app_pc = instr_get_app_pc(instr);
        uint32_t current_location = (uint32_t)(uintptr_t)app_pc; // ((uint32_t)(uintptr_t)app_pc * (uint32_t)33533) & 0xFFFF;
        dr_printf("curr_loc: %x\n", current_location);
        // теперь делаем трейс
        /*
         * current_location = <LOCATION_IDENTIFIER>
         * shared_memory[current_locarion ^ previous_location]++
         * previous_location = current_location >> 1
         * 
         * mov rdx, tls->location
         * xor rdx, current_location
         * inc tls->data[rdx]
         */
        instr_t * instr_1 = XINST_CREATE_move(  drcontext, 
                                                opnd_create_reg(tls_loc_rax), 
                                                OPND_CREATE_MEM64(tls_data_reg, offsetof(thread_data, location)));
        instr_t * instr_2 = INSTR_CREATE_xor(   drcontext,
                                                opnd_create_reg(tls_loc_rax), 
                                                OPND_CREATE_INT32(current_location));
        instr_t * instr_3 = INSTR_CREATE_xor(   drcontext,
                                                opnd_create_reg(tls_loc_rdx), 
                                                opnd_create_reg(tls_loc_rdx));
        instr_t * instr_4 = INSTR_CREATE_mov_imm(  drcontext,
                                                opnd_create_reg(ebx), 
                                                OPND_CREATE_INT32(MAP_SIZE));
        instr_t * instr_5 = INSTR_CREATE_div_4( drcontext,
                                                opnd_create_reg(DR_REG_EBX));
        instr_t * instr_6 = INSTR_CREATE_inc(   drcontext, 
                                                opnd_create_base_disp(  tls_data_reg, 
                                                                        tls_loc_rdx, 
                                                                        1, 
                                                                        offsetof(thread_data, map), 
                                                                        OPSZ_1));
        instr_t * instr_7 = XINST_CREATE_store( drcontext,
                                                OPND_CREATE_MEM64(tls_data_reg, offsetof(thread_data, location)),
                                                OPND_CREATE_INT32(current_location >> 1));

        // вставляем инструкции в код
        instrlist_meta_preinsert(bb, instr, instr_1);
        instrlist_meta_preinsert(bb, instr, instr_2);
        instrlist_meta_preinsert(bb, instr, instr_3);
        instrlist_meta_preinsert(bb, instr, instr_4);
        instrlist_meta_preinsert(bb, instr, instr_5);
        instrlist_meta_preinsert(bb, instr, instr_6);
        instrlist_meta_preinsert(bb, instr, instr_7);

        // возвращаем регистры и флаги на место
        dr_restore_reg(drcontext, bb, instr, ebx, SPILL_SLOT_5);
        dr_restore_reg(drcontext, bb, instr, tls_loc_rax, SPILL_SLOT_4);
        dr_restore_reg(drcontext, bb, instr, tls_loc_rdx, SPILL_SLOT_3);
        dr_restore_reg(drcontext, bb, instr, tls_data_reg, SPILL_SLOT_2);
        dr_restore_arith_flags(drcontext, bb, instr, SPILL_SLOT_1);
    }

    return DR_EMIT_DEFAULT;
}

std::vector<instr_t *> construct_tracer(void *   drcontext, 
                                        reg_id_t reg_flags, 
                                        reg_id_t reg1, 
                                        reg_id_t reg2, 
                                        reg_id_t reg3, 
                                        size_t current_location,
                                        size_t trace_size,
                                        size_t trace_address)
{
    /*
     * reg_flag должен содержать flags
     * остальные регистры должны быть отличны от reg_flag
     * последний регистр должен быть rdx
     */
    reg_id_t xax = reg_flags, xbx = reg1, xcx = reg2, xdx = reg3;

    instr_t * instr_1 = XINST_CREATE_move(      drcontext, 
                                                opnd_create_reg(xcx),
                                                opnd_create_reg(xax));
    instr_t * instr_2 = INSTR_CREATE_mov_imm(   drcontext,
                                                opnd_create_reg(xax), 
                                                OPND_CREATE_INT32(current_location));
    instr_t * instr_3 = INSTR_CREATE_mov_imm(   drcontext,
                                                opnd_create_reg(xbx), 
                                                OPND_CREATE_INT32(trace_size));
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

void insert_instructions(void * drcontext, instrlist_t * bb, instr_t * where, std::vector<instr_t *> new_instructions) {
    for (auto & instr : new_instructions) {
        instrlist_meta_preinsert(bb, where, instr);
    }
}

static dr_emit_flags_t
bb_instrumentation_event_handler(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, bool for_trace, bool translating, void *user_data)
{
    FILE *log = (FILE *)(ptr_uint_t)drmgr_get_tls_field(drcontext, tls_log_ind);

    app_pc bb_addr_1 = dr_fragment_app_pc(tag);
    app_pc bb_addr_2 = dr_app_pc_for_decoding(bb_addr_1);

    if ((size_t)func_bounds.first <= (size_t)bb_addr_2 && (size_t)bb_addr_2 <= (size_t)func_bounds.second)
    {
        reg_id_t xcx = DR_REG_XCX, xdx = DR_REG_XDX, xax = DR_REG_XAX, xbx = DR_REG_XBX;

        // сохраняем регистры и флаги
        dr_save_arith_flags(drcontext, bb, instr, SPILL_SLOT_1);
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
        size_t start_size_t = (size_t) start_local + (size_t) pc;
        size_t stop_size_t = (size_t) stop_local + (size_t) pc;

        auto new_bb = construct_tracer(drcontext, DR_REG_EAX, DR_REG_EBX, DR_REG_ECX, DR_REG_EDX, current_location, stop-start-8, start_size_t);
        insert_instructions(drcontext, bb, instr, new_bb);

        // возвращаем регистры и флаги на место
        dr_restore_reg(drcontext, bb, instr, xbx, SPILL_SLOT_5);
        dr_restore_reg(drcontext, bb, instr, xax, SPILL_SLOT_4);
        dr_restore_reg(drcontext, bb, instr, xdx, SPILL_SLOT_3);
        dr_restore_reg(drcontext, bb, instr, xcx, SPILL_SLOT_2);
        dr_restore_arith_flags(drcontext, bb, instr, SPILL_SLOT_1);
    }

    return DR_EMIT_DEFAULT;
}

static void
exit_event(void)
{
    drmgr_exit();
}
static void
event_thread_init(void *drcontext)
{
    dr_printf("<<<<< new thread init >>>>>\n");

    // для логов
    FILE *log_file = fopen("DynamoRIO_log_file.txt", "w+");
    fseek(log_file, 0, SEEK_SET);
    drmgr_set_tls_field(drcontext, tls_log_ind, (void *)(ptr_uint_t)log_file);

    // для хранения данных потока в tls хранилище
    void * data = dr_thread_alloc(drcontext, sizeof(thread_data));
    memset(data, 0, sizeof(thread_data));
    drmgr_set_tls_field(drcontext, tls_data_ind, data);
}
static void
event_thread_exit(void *drcontext)
{
    dr_printf("<<<<< thread exit >>>>>\n");
    
    // закрываем логи
    fclose((FILE *)(ptr_uint_t)drmgr_get_tls_field(drcontext, tls_log_ind));

    // чистим данные потока
    thread_data *data = (thread_data *) drmgr_get_tls_field(drcontext, tls_data_ind);
    dr_mutex_lock(lock);
    // какие-то финальные действия с данными потока
    dr_printf("####################### DATA ########################\n");
    for (size_t i = 0; i < MAP_SIZE; ++i) {
        dr_printf("%c", data->map[i]);
    }
    dr_printf("\n####################### DATA ########################\n");
    dr_mutex_unlock(lock);
    dr_thread_free(drcontext, data, sizeof(thread_data));
}

void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    lock = dr_mutex_create();
    if (!drmgr_init())
        throw std::runtime_error("cannot init dr_mgr");
    // callback на завершении исполнения
    dr_register_exit_event(exit_event);

    std::ofstream out("DymanoRIO_cout.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
    std::cout.rdbuf(out.rdbuf());                // redirect std::cout to out.txt!

    // хранилище
    tls_log_ind = drmgr_register_tls_field();
    tls_data_ind = drmgr_register_tls_field();
    tls_tracer_ind = drmgr_register_tls_field();
    if (!drmgr_register_thread_init_event(event_thread_init) ||
        !drmgr_register_thread_exit_event(event_thread_exit))
    {
        throw std::runtime_error("tls trubles appeared");
    }

    // если всё норм - достаём адреса начала и конца
    func_bounds = get_func_bounds_gpa("fuzz_app", "_asm_sort");
    std::cout << "_asm_sort" << "\t----->\t" << (size_t)func_bounds.first << " - " << (size_t)func_bounds.second << std::endl;
    std::cout << "_asm_sort" << "\t----->\t" << std::hex << (size_t)func_bounds.first << " - " << (size_t)func_bounds.second << std::endl;
    start = (size_t) func_bounds.first;
    stop = (size_t) func_bounds.second;

    start_local = get_symbol_offset("fuzz_app", "__start___libfuzzer_extra_counters");
    stop_local  = get_symbol_offset("fuzz_app", "__stop___libfuzzer_extra_counters");

    // привязываем обработчик к функции
    drmgr_register_bb_instrumentation_event(NULL, bb_instrumentation_event_handler, NULL);
}