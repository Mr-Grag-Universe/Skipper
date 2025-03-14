#include <thread>
#include <stdexcept>

#define X86_64
// #define X64
#include "dr_api.h"
#include "dr_tools.h"
#include "dr_events.h"
#include "dr_os_utils.h"
#include "drmgr.h"
#include "drreg.h"
#include "drx.h"

#include "include/func_bounds.h"
#include "include/funcs.h"
#include "include/debug.h"

#include "include/classes/Config.h"
#include "include/classes/Logger.h"
#include "include/classes/Tracer.h"
#include "include/classes/Guarder.h"
#include "include/classes/Options.h"

#include "loggers.h"

using namespace std::chrono_literals;

static Configurator config;
static Tracer tracer;
static std::vector <CodeSegmentDescriber> code_segment_describers;
static std::set<int> opcodes;
static Guarder guarder;

static std::map<std::string, std::vector <long long int>> global_guards;

static int tls_key;

void init_tls() {
    tls_key = drmgr_register_tls_field();
    if (tls_key == -1) {
        throw std::runtime_error("cannot allocate tls!");
    }
    dr_printf("thread <%s> tls inited!\n", get_thread_id().c_str());
}

bool address_in_code_segment(void * tag, std::vector <CodeSegmentDescriber> & segments) {
    app_pc 
    bb_addr = dr_fragment_app_pc(tag);
    bb_addr = dr_app_pc_for_decoding(bb_addr);

    for (auto & segment : segments) {
        if (segment.start <= (size_t)bb_addr && (size_t)bb_addr <= segment.end) {
            main_logger.log("INSTR", "addr = ", int_to_hex((size_t) tag) + " : " + int_to_hex(segment.start) + " - " + int_to_hex(segment.end));
            return true;
        }
    }
    return false;
}


bool address_in_global_guard(void * drcontext, instrlist_t * bb, void *tag, instr_t * c_instr) {
    bool guards_opened = false;
    bool good_lea_found = false;
    for (auto instr = instrlist_first(bb); instr != NULL; instr = instr_get_next(instr)) {
        if (instr == c_instr) {
            return guards_opened;
        }
        print_instruction(drcontext, instr);

        int opcode = instr_get_opcode(instr);
        if (opcode == (int) OP_lea) {
            opnd_t src = instr_get_src(instr, 0);
            if (!opnd_is_memory_reference(src)) {
            }


            void* mem_addr;
            try {
                main_logger.log("INSTR", "1 opnd get addr");
                mem_addr = opnd_get_addr(src);
            } catch (...) {
                good_lea_found = false;
                continue;
            }
            main_logger.log("INSTR", "mem_addr: {}", (long) mem_addr);

            if (std::find(  global_guards["fuzz_app"].begin(), 
                            global_guards["fuzz_app"].end(), 
                            (long long) mem_addr) != global_guards["fuzz_app"].end()) {
                good_lea_found = true;
                continue;
            }
        } else if (good_lea_found && instr_writes_memory(instr)) {
            print_instruction(drcontext, instr);
            opnd_t src = instr_get_src(instr, 0);
            if (opnd_is_immed_int(src)) {
                int val = opnd_get_immed_int(src);
                main_logger.log("INSTR", "move opnd value is {}", val);
                guards_opened = (val == 1);

                if (guards_opened)
                    main_logger.log("INSTR", "open the gates!");
                else
                    main_logger.log("INSTR", "close the gates!");
            }
        }
        good_lea_found = false;
    }
    return guards_opened;
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

    int op = instr_get_opcode(instr);
    if (opcodes.find(op) != opcodes.end()) {
        if (address_in_code_segment(tag, code_segment_describers)) {
            tracer.traceOverflow(drcontext, tag, bb, instr);

            // логируем
            char buff[1024];
            instr_disassemble_to_buffer(drcontext, instr, buff, 1024);
            main_logger.log("INSTR", "addr = {}", int_to_hex((size_t) instr_get_app_pc(instr)));
            main_logger.log("INSTR", std::string(buff));
        } else if (guarder.guards_opened) {
            tracer.traceOverflow(drcontext, tag, bb, instr);
        }
    }
    guarder.throw_instr(instr);

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
    dr_printf("thread_id: %s\n", get_thread_id().c_str());
    Logger* logger = new Logger(create_log_file_name(thread_id));
    drmgr_set_tls_field(drcontext, tls_key, logger);
}
static void
exit_event(void)
{
    dr_printf(">>>>> exit event <<<<<\n");
    dr_printf("thread_id: %s\n", get_thread_id().c_str());
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
    dr_printf("thread_id: %s\n", get_thread_id().c_str());

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
    dr_printf("thread_id: %s\n", get_thread_id().c_str());
}

static void 
event_module_load(void *drcontext, const module_data_t *mod, bool loaded) {
    if (loaded) {
    }
}

void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    // parsing command line arguments
    Parser parser;
    auto ptr = std::make_shared<Option<std::string>>("config", "input/settings.json", "d1", "d2");
    parser.add_option(ptr);
    std::string path_to_config = "input/settings.openssl.json";
    if (parser.parse_argv(argc, argv, NULL, NULL) && parser["config"]->is_specified()) {
        path_to_config = parser["config"]->get_value_str();
    }

    // load configuration file settings
    config.load_config(path_to_config);
    // setup logging
    if (config.logFuzzingEnabled()) {
        auto path = config.getLogFuzzingPath();
        main_logger.set_log_file(path);
        main_logger.start_logging("", true);
    }
    // setup tracer for work
    tracer.set_config(config);

    if (config.debugModeEnabled()) {
        main_logger.log_program_params();
    }

    auto tid = get_thread_id();
    if (!drmgr_init())
        throw std::runtime_error("cannot init dr_mgr");

    //  проверка, что мы в той программе
    // если это не программа с искомым модулем - не исполняемся дальше
    auto analized_modules_names = config.get_modules_names();
    auto current_modules_names = get_modules_names();
    if (config.debugModeEnabled()) {
        for (auto & m: analized_modules_names) {
            main_logger.log_debug("{} : analized module name: {}", tid, m);
        }
        for (auto & m: current_modules_names) {
            main_logger.log_debug("{} : visible module name: {}", tid, m);
        }
    }
    std::set <std::string> cmn_set(current_modules_names.begin(), current_modules_names.end());
    if (!std::includes( cmn_set.begin(), cmn_set.end(), 
                        analized_modules_names.begin(), analized_modules_names.end())) {
                            dr_printf("[WARNING] : %s : there is not modules in current process!\n", tid.c_str());
                            main_logger.log_warning("{} : there is not modules in current process!", tid);
                            return;
                        }

    // ищем guards по всем доступным модулям
    for (auto & module_name : cmn_set) {
        module_data_t * module = dr_lookup_module_by_name(module_name.c_str());
        if (module) {
            long long global_var_addr = (long long) dr_get_proc_address(module->handle, "instr_global");
            if (global_var_addr == 0) {
                main_logger.log_info("{} : global guard var in module <{}> was not found! =(", tid, module_name);
            } else {
                dr_printf("[INFO] : %s : global guard var in module <%s>: %ld, base: %ld\n", tid.c_str(), module_name.c_str(), (long long) global_var_addr, (long long) module->start);
                main_logger.log_info("{} : global guard var in module <{}>: {}, base: {}", tid, module_name, (long long) global_var_addr, (long long) module->start);
                global_guards[module_name].push_back((long long int) global_var_addr);
            }
            dr_free_module_data(module);
        } else {
            // error
            main_logger.log_error("cannot find one of previosly seen modules <{}>", module_name);
            exit(1);
        }
    }
    guarder.set_global_guards(global_guards);
    dr_printf("[INFO] : %s : global guard vars collected!\n", tid.c_str());
    main_logger.log_info("{} : global guard vars collected!", tid);
    
    opcodes = config.getInspectOpcodes();

    if (config.debugModeEnabled()) {
        main_logger.log_modules();
        print_modules();
    }

    std::set<std::string> symbols;
    Logger symbol_logger;
    if (config.logSymbolsEnabled()) {
        auto path = config.getLogSymbolsPath();
        symbol_logger.set_log_file(path);
        symbol_logger.start_logging();
    }
    if (config.debugModeEnabled()) {
        main_logger.log_debug("symbol_logger activated");
    }

    std::map<std::string, FuncConfig>
    inspect_functions = config.getInspectionFunctions();

    if (!inspect_functions.empty()) {
        auto symbols_map = get_all_symbols_with_offsets(
                                        (*inspect_functions.begin()).second.module_name, 
                                        (*inspect_functions.begin()).second.module_path, 
                                        config.getFuzzConfig()["use_pattern"]);
        for (auto & symbol : symbols_map) {
            if (config.logSymbolsEnabled()) {
                std::ostringstream oss;
                oss << symbol.first << " : " << std::hex << (size_t) symbol.second;
                symbol_logger.log_debug(oss.str());
            }
        }

        
        auto func_bounds = get_func_bounds(inspect_functions, true, config.use_default_bounds());
        for (auto & func : func_bounds) {
            if (config.logSymbolsEnabled()) {
                std::ostringstream oss;
                oss << func.first << " : " << std::hex << (size_t) func.second.first << " - " << (size_t) func.second.second;
                symbol_logger.log_debug(oss.str());
            }

            code_segment_describers.push_back({(size_t) func.second.first, (size_t) func.second.second});
        }
        symbol_logger.stop_logging();
        if (!symbol_logger.is_open()) {
            main_logger.log_info("log_symbols_stream closed successfuly!");
        } else {
            main_logger.log_error("log_symbols_stream cannot be closed successfuly!");
            dr_printf("log_symbols_stream cannot be closed successfuly!\n");
            dr_abort();
        }
        main_logger.log_info("symbols logged! log stream closed.");
    }
    // предупреждаем, если совсем ничего не нашли
    if (code_segment_describers.size() == 0) {
        main_logger.log_warning("there is not no one symbol from inspection function names passed to fuzzer!");
        dr_printf("[WARNING] : there is not no one symbol from inspection function names passed to fuzzer!\n");
    }
    main_logger.log_info("{} : DR segments readed successfully!", tid);

    // достаём адрес для доп покрытия
    for (auto & p : config.get_modules_info()) {
        auto module_name = p.first;
        auto module_path = p.second;
        size_t extra_counters_start = get_symbol_offset(module_name, 
                                                        module_path, 
                                                        "__start___libfuzzer_extra_counters");
        size_t extra_counters_stop  = get_symbol_offset(module_name, 
                                                        module_path, 
                                                        "__stop___libfuzzer_extra_counters");

        if (extra_counters_start && extra_counters_stop) {
            tracer.set_trace_area(extra_counters_start, extra_counters_stop);
            break;
        }
    }
    main_logger.log_info("{} : extra_counters address found successfully!", tid);
    
    main_logger.log_info("{} : DR configured!", tid);
    main_logger.log_line();
    dr_printf("%s : DR configured!\n", tid.c_str());

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

    main_logger.log("SYS", "{} : main_client function work is complete!", tid);
    dr_printf("[SYS] : %s : main_client function work is complete!\n", tid.c_str());
}
