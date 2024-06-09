#include <thread>
#include <stdexcept>

#define X86
#include "dr_api.h"
#include "dr_tools.h"

#include "include/func_bounds.h"
#include "include/funcs.h"

#include "Config.h"
#include "Logger.h"
#include "Tracer.h"

using namespace std::chrono_literals;

static Configurator config(std::string("input/settings.json"));
static Tracer tracer(config);
static std::vector <CodeSegmentDescriber> code_segment_describers;
static Logger main_logger;


bool address_in_code_segment(void * tag, std::vector <CodeSegmentDescriber> & segments)
{ 
    app_pc 
    bb_addr = dr_fragment_app_pc(tag);
    bb_addr = dr_app_pc_for_decoding(bb_addr);

    for (auto & segment : segments) {
        if (segment.start <= (size_t)bb_addr && (size_t)bb_addr <= segment.end) {
            main_logger.log("ADDR", int_to_hex((size_t) tag) + " : " + int_to_hex(segment.start) + " - " + int_to_hex(segment.end));
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
        int op = instr_get_opcode(instr);
        if (op == OP_add || op == OP_adc || op == OP_sub || op == OP_sbb) {
            tracer.traceOverflow(drcontext, tag, bb, instr);
        }

        // логируем
        char buff[1024];
        instr_disassemble_to_buffer(drcontext, instr, buff, 1024);
        main_logger.log("ADDR", int_to_hex((size_t) instr_get_app_pc(instr)));
        main_logger.log("INSTR", std::string(buff));
    }

    return DR_EMIT_DEFAULT;
}

static void
exit_event(void)
{
    main_logger.stop_logging();
    drmgr_exit();
}
static void
event_thread_init(void *drcontext)
{
    dr_printf("<<<<< new thread init >>>>>\n");
}
static void
event_thread_exit(void *drcontext)
{
    dr_printf("<<<<< thread exit >>>>>\n");
}

void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    if (!drmgr_init())
        throw std::runtime_error("cannot init dr_mgr");

    tracer.set_registers(DR_REG_EAX, {DR_REG_EBX, DR_REG_ECX, DR_REG_EDX});
    dr_printf("DR configured\n");

    print_modules();

    std::set<std::string> symbols;
    Logger symbol_logger;
    if (config.logSymbolsEnabled()) {
        auto path = config.getLogSymbolsPath();
        symbol_logger.set_log_file(path);
        symbol_logger.start_logging();
    }
    if (config.logFuzzingEnabled()) {
        auto path = config.getLogFuzzingPath();
        main_logger.set_log_file(path);
        main_logger.start_logging();
    }

    std::map<std::string, FuncConfig>
    inspect_functions = config.getInspectionFunctions();

    auto symbols_map = get_all_symbols_with_offsets(
                                    (*inspect_functions.begin()).second.module_name, 
                                    (*inspect_functions.begin()).second.module_path, 
                                    config.getFuzzConfig()["use_pattern"]);
    Logger debug_logger;
    debug_logger.set_log_file("out/logs/debug_logs.txt");
    debug_logger.start_logging();
    debug_logger.stop_logging();

    for (auto & symbol : symbols_map) {
        if (config.logSymbolsEnabled()) {
            std::ostringstream oss;
            oss << symbol.first << " : " << std::hex << (size_t) symbol.second;
            symbol_logger.log("DEBUG", oss.str());
        }
    }

    
    auto func_bounds = get_func_bounds_optimized(inspect_functions, true, config.use_default_bounds());
    for (auto & func : func_bounds) {
        if (config.logSymbolsEnabled()) {
            std::ostringstream oss;
            oss << func.first << " : " << std::hex << (size_t) func.second.first << " - " << (size_t) func.second.second;
            symbol_logger.log("DEBUG", oss.str());
        }

        code_segment_describers.push_back({(size_t) func.second.first, (size_t) func.second.second});
    }
    // предупреждаем, если совсем ничего не нашли
    if (code_segment_describers.size() == 0) {
        dr_printf("[WARNING] : there is not no one symbol from inspection function names passed to fuzzer!\n");
    }
    symbol_logger.stop_logging();
    if (!symbol_logger.is_open()) {
        dr_printf("[INFO] : log_symbols_stream closed successfuly!\n");
    } else {
        dr_printf("[ERROR] : log_symbols_stream cannot be closed successfuly!\n");
        dr_abort();
    }

    dr_printf("[INFO] : symbols logged! log stream closed.\n");
    dr_printf("DR segments readed successfully!\n");
    fflush(stdout);
    
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
