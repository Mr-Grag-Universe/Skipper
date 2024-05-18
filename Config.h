#ifndef MY_CONFIG_header
#define MY_CONFIG_header

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


class Configurator {
private:
    void config_with_json(json config_data) {
        
        // имена инспетируемых функций
        // auto inspect_functions = config_data["inspect_functions"];
        // if (this->inspect_functions.empty()) {
        //     this->inspect_functions = std::set<std::string>();
        // }
        // for (json::iterator iter = inspect_functions.begin(); iter != inspect_functions.end(); ++iter) {
        //     std::cout << *iter << std::endl;
        //     // this->inspect_functions.insert(iter)
        // }

        // настройки инспектирования asm-вставок
        // TODO

        // настройки tracer
        this->tracer_config = config_data["tracer_config"];
    }

public:
    json tracer_config;
    json fuzzing_config;
    json _config;

    Configurator(const std::string config_file_name = "settings.json") {
        std::cout << "loading " << config_file_name << "..." << std::endl;
        std::ifstream f(config_file_name, std::ifstream::in);
        
        std::cout << "parsing..." << std::endl;
        json config_data;
        f >> config_data;

        std::cout << "assigning..." << std::endl;
        // this->config_with_json(config_data);
        this->_config = config_data;

        std::cout << "configured" << std::endl;
    }

    void load_config(std::string config_file_name = "settings.json") {
        std::ifstream f(config_file_name);
        json config_data = json::parse(f);
        this->_config = config_data;
        // this->config_with_json(config_data);
    }

    void config(json config_data) {
        this->_config = config_data;
        // this->config_with_json(config_data);
    }
    void config(std::string config_string) {
        json config_data = json::parse(config_string);
        // this->config_with_json(config_data);
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
    std::vector<std::map<std::string, std::string>> 
    getInspectionFunctions() const {
        std::cout << "inspection functions providing..." << std::endl;
        auto f_info_json = this->_config["fuzzing"]["inspect_funcs"];
        std::vector<std::map<std::string, std::string>> 
        f_names;
        for (auto f_info : f_info_json) {
            f_names.push_back({
                                {"module_name",     f_info["module_name"]}, 
                                {"module_path",     f_info["module_path"]}, 
                                {"func_name",       f_info["func_name"]},
                                {"default_start",   f_info["default_address"]["start"]},
                                {"default_stop",    f_info["default_address"]["stop"]}
                            });
        }
        std::cout << "functions collected successfuly!" << std::endl;
        return f_names;
    }

    bool
    logSymbolsEnabled() const {
        return this->_config["logging"]["log_symbols"]["enable"];
    }
    std::string
    getLogSymbolsPath() const {
        bool enable = this->_config["logging"]["log_symbols"]["enable"];
        if (!enable) {
            std::cout << "[WARNING] : symbols logging is disabled" << std::endl;
        }
        return this->_config["logging"]["log_symbols"]["path"];
    }

    bool
    logFuzzingEnabled() const {
        return this->_config["logging"]["log_fuzzing"]["enable"];
    }
    std::string
    getLogFuzzingPath() const {
        bool enable = this->_config["logging"]["log_fuzzing"]["enable"];
        if (!enable) {
            std::cout << "[WARNING] : fuzzing logging is disabled" << std::endl;
        }
        return this->_config["logging"]["log_fuzzing"]["path"];
    }
};

#endif // MY_CONFIG_header