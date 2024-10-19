#include <thread>
#include <stdexcept>

#define X86
#include "dr_api.h"
#include "dr_tools.h"
#include "dr_events.h"

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
static std::set<int> opcodes;

static int tls_key;

void init_tls() {
    tls_key = drmgr_register_tls_field();
    if (tls_key == -1) {
        throw std::runtime_error("cannot allocate tls!");
    }
    dr_printf("thread tls inited!\n");
}

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
bb_instrumentation_event_handler(
                                    void *drcontext, 
                                    void *tag, 
                                    instrlist_t *bb, 
                                    instr_t *instr, 
                                    bool for_trace, 
                                    bool translating, 
                                    void *user_data)
{
    app_pc bb_addr_1 = dr_fragment_app_pc(tag);
    app_pc bb_addr_2 = dr_app_pc_for_decoding(bb_addr_1);

    if (address_in_code_segment(tag, code_segment_describers))
    {
        int op = instr_get_opcode(instr);
        if (opcodes.find(op) != opcodes.end()) {
            // tracer.traceOverflow(drcontext, tag, bb, instr);
        }

        // логируем
        char buff[1024];
        instr_disassemble_to_buffer(drcontext, instr, buff, 1024);
        main_logger.log("ADDR", int_to_hex((size_t) instr_get_app_pc(instr)));
        main_logger.log("INSTR", std::string(buff));
    }

    return DR_EMIT_DEFAULT;
}

std::string create_log_file_name(std::thread::id id) {
    std::ostringstream oss;
    oss << "./out/logs/log-" << std::hash<std::thread::id>()(id) << ".txt";
    return oss.str();
}

static void
event_thread_init(void *drcontext)
{
    dr_printf("<<<<< new thread init >>>>>\n");
    init_tls();
    std::thread::id thread_id = std::this_thread::get_id();
    dr_printf("thread_id: %u\n", thread_id);
    Logger* logger = new Logger(create_log_file_name(thread_id));
    drmgr_set_tls_field(drcontext, tls_key, logger);
}
static void
exit_event(void)
{
    dr_printf(">>>>> exit event <<<<<\n");
    if (!drmgr_unregister_thread_init_event(event_thread_init) || !drmgr_unregister_bb_instrumentation_event((drmgr_analysis_cb_t) bb_instrumentation_event_handler)) {
            // DR_ASSERT(false);
            dr_printf("STOOOOOOP!!!!!\n");
    }

    main_logger.stop_logging();
    drmgr_exit();
    // мб надо обернуть в try
    drsym_exit();

    dr_printf("abording!!!\n");
    dr_abort_with_code(111);
}
static void
event_thread_exit(void *drcontext)
{
    dr_printf("<<<<< thread exit >>>>>\n");
    // dr_app_cleanup();
    // dr_app_stop_and_cleanup();

    dr_printf("thread tls clearing...\n");
    Logger* logger = static_cast<Logger*>(drmgr_get_tls_field(drcontext, tls_key));
    if (logger != NULL) {
        delete logger; // Освобождение ресурсов логгера
        drmgr_set_tls_field(drcontext, tls_key, NULL); // Удаление из TLS
    }
}

static void
fork_init_event(void *drcontext)
{
    dr_printf("<<<<< fork init >>>>>\n");
}

void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_printf("hellow world!\n");
    if (!drmgr_init())
        throw std::runtime_error("cannot init dr_mgr");

    //  проверка, что мы в той программе
    // если это не программа с искомым модулем - не исполняемся дальше
    auto analized_modules_names = config.get_modules_names();
    auto current_modules_names = get_modules_names();
    std::set <std::string> cmn_set(current_modules_names.begin(), current_modules_names.end());
    if (!std::includes( cmn_set.begin(), cmn_set.end(), 
                        analized_modules_names.begin(), analized_modules_names.end())) {
                            dr_printf("[INFO] : there is not modules in current process!\n");
                            return;
                        }
    
    tracer.set_registers(DR_REG_EAX, {DR_REG_EBX, DR_REG_ECX, DR_REG_EDX});
    opcodes = config.getInspectOpcodes();
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

    dr_register_fork_init_event((void (*)(void *)) fork_init_event);

    drmgr_register_bb_instrumentation_event(NULL, bb_instrumentation_event_handler, NULL);


    dr_printf("[SYS] : sleeping!\n");
    std::this_thread::sleep_for(5s);
}
