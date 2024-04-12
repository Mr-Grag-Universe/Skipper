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
        auto inspect_functions = config_data["inspect_functions"];
        if (this->inspect_functions.empty()) {
            this->inspect_functions = std::set<std::string>();
        }
        for (json::iterator iter = inspect_functions.begin(); iter != inspect_functions.end(); ++iter) {
            std::cout << *iter << std::endl;
            // this->inspect_functions.insert(iter)
        }

        // настройки инспектирования asm-вставок
        // TODO

        // настройки tracer
        this->tracer_config = config_data["tracer_config"];
    }

public:
    std::set<std::string> inspect_functions;
    json tracer_config;

    Configurator(const std::string config_file_name = "settings.json") {
        std::cout << "loading " << config_file_name << "..." << std::endl;
        std::ifstream f(config_file_name, std::ifstream::in);
        
        std::cout << "parsing..." << std::endl;
        json config_data;
        f >> config_data;

        std::cout << "assigning..." << std::endl;
        this->config_with_json(config_data);

        std::cout << "configured" << std::endl;
    }

    void load_config(std::string config_file_name = "settings.json") {
        std::ifstream f(config_file_name);
        json config_data = json::parse(f);
        this->config_with_json(config_data);
    }

    void config(json config_data) {
        this->config_with_json(config_data);
    }
    void config(std::string config_string) {
        json config_data = json::parse(config_string);
        this->config_with_json(config_data);
    }

    // Раздача конфигов
    json getTracerConfig() const {
        return this->tracer_config;
    }
};

#endif // MY_CONFIG_header