#ifndef MY_CONFIG_header
#define MY_CONFIG_header

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "../types.h"
#include "../../loggers.h"

using json = nlohmann::json;


class Configurator {
private:
    void config_with_json(json config_data) {
        // настройки инспектирования asm-вставок
        // TODO

        // настройки tracer
        this->tracer_config = config_data["tracer_config"];
    }

    int get_opcode(std::string op_s) const {
        if (op_s == "add") {
            return OP_add;
        } else if (op_s == "adc") {
            return OP_adc;
        } else if (op_s == "sub") {
            return OP_sub;
        } else if (op_s == "sbb") {
            return OP_sbb;
        } else if (op_s == "adcx") {
            return OP_adcx;
        } else if (op_s == "adox") {
            return OP_adox;
        } else if (op_s == "cmp") {
            return OP_cmp;
        } else if (op_s == "movq") {
            return OP_movq;
        } else {
            throw std::runtime_error("we do not know this opcode's name!");
            return -1;
        }
    }
public:
    json tracer_config;
    json fuzzing_config;
    json _config;

    Configurator(const std::string config_file_name = "") {
        if (!config_file_name.empty()) {
            this->load_config(config_file_name);
        }
    }

    void load_config(std::string config_file_name = "settings.json") {
        std::cout << "loading " << config_file_name << "..." << std::endl;
        std::ifstream f(config_file_name, std::ifstream::in);
        json config_data;
        f >> config_data;
        this->_config = config_data;
        std::cout << "configuration loaded!" << std::endl;
    }

    void config(json config_data) {
        this->_config = config_data;
    }
    void config(std::string config_string) {
        json config_data = json::parse(config_string);
        this->_config = config_data;
    }

    // Раздача конфигов
    json getTracerConfig() const {
        return this->_config["tracer"];
    }

    // Настройка логики фаззинга
    json getFuzzConfig() const {
        return this->_config["fuzzing"];
    }

    // возвращаем имена функций, обязательные для фаззинга
    std::map<std::string, FuncConfig> 
    getInspectionFunctions() const {
        std::cout << "inspection functions providing..." << std::endl;
        auto f_info_json = this->_config["fuzzing"]["inspect_funcs"];
        std::map<std::string, FuncConfig> 
        f_names;
        for (auto f_info : f_info_json) {
            FuncConfig conf =   {
                                    f_info["module_name"],
                                    f_info["module_path"],
                                    std::make_pair(
                                        (size_t) std::stoull((std::string) f_info["default_address"]["start"], nullptr, 16), 
                                        (size_t) std::stoull((std::string) f_info["default_address"]["start"], nullptr, 16))
                                };
            f_names[f_info["func_name"]] = conf;
        }
        std::cout << "functions collected successfuly!" << std::endl;
        return f_names;
    }

    bool
    logSymbolsEnabled() const {
        if (!_config.contains("logging")) {
            throw std::runtime_error("Missing 'logging' section");
        }
        return this->_config["logging"]["log_symbols"]["enable"];
    }
    std::string
    getLogSymbolsPath() const {
        if (!_config.contains("logging")) {
            throw std::runtime_error("Missing 'logging' section");
        }
        bool enable = this->_config["logging"]["log_symbols"]["enable"];
        if (!enable) {
            std::cout << "[WARNING] : symbols logging is disabled" << std::endl;
        }
        return this->_config["logging"]["log_symbols"]["path"];
    }

    bool
    logFuzzingEnabled() const {
        if (!_config.contains("logging")) {
            throw std::runtime_error("Missing 'logging' section");
        }
        return this->_config["logging"]["log_fuzzing"]["enable"];
    }
    std::string
    getLogFuzzingPath() const {
        if (!_config.contains("logging")) {
            throw std::runtime_error("Missing 'logging' section");
        }
        bool enable = this->_config["logging"]["log_fuzzing"]["enable"];
        if (!enable) {
            std::cout << "[WARNING] : fuzzing logging is disabled" << std::endl;
        }
        return this->_config["logging"]["log_fuzzing"]["path"];
    }

    std::string
    getFuzzingCorpusPath() const {
        return this->_config["fuzzing"]["corpus_path"];
    }

    bool
    use_default_bounds() const {
        return this->_config["fuzzing"]["use_default"];
    }

    std::set<int> getInspectOpcodes() const {
        dr_printf("getting opcodes to inspect...");
        std::set<int> ops;
        for (auto op : this->_config["fuzzing"]["inspect_opcodes"]) {
            ops.insert(this->get_opcode((std::string) op));
        }
        return ops;
    }

    std::set <std::string> get_modules_names() const {
        main_logger.log_info("getting modules names...");
        std::set <std::string> module_names;
        for (auto mn : this->_config["fuzzing"]["inspect_funcs"]) {
            std::string name = mn["module_name"];
            module_names.insert((std::string) name);
        }
        return module_names;
    }

    std::set <std::pair<std::string, std::string>> get_modules_info() const {
        dr_printf("getting modules names...\n");
        std::set <std::pair<std::string, std::string>> modules_info;
        for (auto mn : this->_config["fuzzing"]["inspect_funcs"]) {
            std::string name = mn["module_name"];
            std::string path = mn["module_path"];
            modules_info.insert(std::make_pair(name, path));
        }
        return modules_info;
    }

    bool debugModeEnabled() const {
        return this->_config["debug"];
    }
};

#endif // MY_CONFIG_header
