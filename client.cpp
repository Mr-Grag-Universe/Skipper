#include <thread>

#define X86
#include "dr_api.h"
#include "dr_tools.h"

#include "include/func_bounds.h"
#include "include/funcs.h"

#include "Config.h"
#include "Tracer.h"

using namespace std::chrono_literals;

static Configurator config(std::string("input/settings.json"));
static Tracer tracer(config);
static std::vector<std::vector<std::string>> inspect_functions = {{"fuzz_app", "bin/fuzz_app", "_asm_sort"}};
static std::string main_module = "fuzz_app";
static std::vector <CodeSegmentDescriber> code_segment_describers;


bool address_in_code_segment(void * tag, std::vector <CodeSegmentDescriber> & segments)
{ 
    app_pc 
    bb_addr = dr_fragment_app_pc(tag);
    bb_addr = dr_app_pc_for_decoding(bb_addr);

    for (auto & segment : segments) {
        if (segment.start <= (size_t)bb_addr && (size_t)bb_addr <= segment.end) {
            return true;
        }
    }
    return false;
}

static dr_emit_flags_t
bb_instrumentation_event_handler(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, bool for_trace, bool translating, void *user_data)
{
    app_pc bb_addr_1 = dr_fragment_app_pc(tag);
    app_pc bb_addr_2 = dr_app_pc_for_decoding(bb_addr_1);

    if (address_in_code_segment(tag, code_segment_describers))
    {
        tracer.trace_instruction(drcontext, tag, bb, instr);
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
    // FILE *log_file = fopen("DynamoRIO_log_file.txt", "w+");
    // fseek(log_file, 0, SEEK_SET);
    // drmgr_set_tls_field(drcontext, tls_log_ind, (void *)(ptr_uint_t)log_file);
}
static void
event_thread_exit(void *drcontext)
{
    dr_printf("<<<<< thread exit >>>>>\n");
    
    // закрываем логи
    // fclose((FILE *)(ptr_uint_t)drmgr_get_tls_field(drcontext, tls_log_ind));
}


void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    if (!drmgr_init())
        throw std::runtime_error("cannot init dr_mgr");

    tracer.set_registers(DR_REG_EAX, {DR_REG_EBX, DR_REG_ECX, DR_REG_EDX});
    dr_printf("DR configured\n");

    for (auto & func : inspect_functions) {
        dr_printf("func_name: %s\n", func[0].c_str());
        auto bounds = get_func_bounds_gpa(func[0], func[1], func[2]);
        size_t func_start = (size_t) bounds.first;
        size_t func_stop = (size_t) bounds.second;
        dr_printf("func_bounds: %zu-%zu \n", func_start, func_stop);
        code_segment_describers.push_back({func_start, func_stop});
    }
    dr_printf("DR segments readed successfully!\n");
    fflush(stdout);
    std::this_thread::sleep_for(1s);

    size_t extra_counters_start = get_symbol_offset("fuzz_app", "bin/fuzz_app", "__start___libfuzzer_extra_counters");
    size_t extra_counters_stop  = get_symbol_offset("fuzz_app", "bin/fuzz_app", "__stop___libfuzzer_extra_counters");
    
    tracer.set_trace_area(extra_counters_start, extra_counters_stop);

    /*
     * ######################## Устанавливаем обработчики ########################3
     */

    dr_register_exit_event(exit_event);

    if (!drmgr_register_thread_init_event(event_thread_init) ||
        !drmgr_register_thread_exit_event(event_thread_exit))
    {
        throw std::runtime_error("thread init | exit events handlers error!\n");
    }

    drmgr_register_bb_instrumentation_event(NULL, bb_instrumentation_event_handler, NULL);
}
