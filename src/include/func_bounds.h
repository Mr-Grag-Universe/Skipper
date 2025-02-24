#ifndef FIND_FUNC_BOUNDS_header
#define FIND_FUNC_BOUNDS_header

#include <algorithm>

#include "dr_tools.h"
#include "drsyms.h"
#include "drmgr.h"
#include "find_pattern.h"
#include "get_all_symbols.h"

#include "types.h"

#include <map>

bool get_func_bounds_callback(drsym_info_t *info, drsym_error_t status, void *data) {
    if (info != NULL && data != NULL) {
        // if (status != DRSYM_SUCCESS) {
        //      printf("line shit is not success!\n");
        // }
        auto * d = (std::vector<std::pair<size_t, std::string>> *) data;
        d->push_back(std::make_pair(info->start_offs, info->name));
    }
    return true;
}

std::map<std::string, std::pair<generic_func_t, generic_func_t>> 
get_func_bounds(std::map<std::string, FuncConfig> inspect_funcs, bool use_pattern, bool use_default_bounds) 
{
    if (inspect_funcs.empty()) {
        dr_printf("[ERROR] : empty instr function map!");
        throw std::invalid_argument("[ERROR] : empty instr function map!");
    }

    // собираем все пары модуль-путь
    std::set<std::pair<std::string, std::string>> module_path;
    for (auto & func : inspect_funcs) {
        module_path.insert(std::make_pair(func.second.module_name, func.second.module_path));
    }
    
    // собираем все символы отовсюду
    // в результате они уже будут указаны с учётом отступа модуля
    std::map<std::string, generic_func_t> symbols;
    for (auto & m_p : module_path) {
        auto symbols_offests = get_all_symbols_with_offsets(m_p.first, m_p.second, use_pattern);
        symbols.merge(symbols_offests);
    }

    // переводим в вектор, чтобы сортировать было удобнее
    std::vector <std::pair<size_t, std::string>> symbols_vector;
    for (auto & symbol : symbols) {
        symbols_vector.push_back({(size_t) symbol.second, symbol.first});
    }

    // сортируем символы
    std::sort(symbols_vector.begin(), symbols_vector.end());

    // для каждой из искомых функций находим границы
    std::map<std::string, std::pair<generic_func_t, generic_func_t>> res;
    for (auto & func : inspect_funcs) {
        std::string func_name = func.first;

        // ищем точное совпадение
        auto iter = std::find_if(symbols_vector.begin(), symbols_vector.end(), [&func_name](const auto x){
            return func_name == std::string(x.second);
        });
        if ((iter == symbols_vector.end()) && use_pattern) {
            dr_printf("second try...\n");
            // ищем неточное совпадение
            iter = std::find_if(
                symbols_vector.begin(), symbols_vector.end(), 
                [&func_name](const auto x){
                    return std::string(x.second).find(func_name) != std::string::npos;
                });
        }
        dr_printf("searching complete!\n");

        if (iter == symbols_vector.end()) {
            dr_printf("cannot find such func_name =(\n");
            char message[1024];
            sprintf(message, "there is not func name <%s> here", func_name.c_str());
            dr_printf("message: %s\n", message);

            std::string answer;
            size_t addr = 0;
            auto default_address = func.second.default_address;
            if (default_address.first && default_address.first <= default_address.second) {
                if (use_default_bounds) {
                    res[func_name] = std::make_pair(
                                                    (generic_func_t) default_address.first, 
                                                    (generic_func_t) default_address.second);
                    continue;
                } else {
                    dr_printf("[CONTROLE] : do you want to use default_address?[y/n] ");
                    std::cin >> answer;
                    if (answer == "y" || answer == "yes") {
                        res[func_name] = std::make_pair(
                                                            (generic_func_t) default_address.first, 
                                                            (generic_func_t) default_address.second);
                        continue;
                    }
                }
            }
            {
                dr_printf("[CONTROLE] : do you want to enter address?[y/n] ");
                std::cin >> answer;
                if (answer == "n" || answer == "no") {
                    res[func_name] = std::make_pair((generic_func_t)0, (generic_func_t)0);
                    continue;
                }
                size_t start{}, stop{};
                dr_printf("[CONTROLE] : enter start address: ");
                std::cin >> start;
                dr_printf("[CONTROLE] : enter stop address: ");
                std::cin >> stop;
                res[func_name] = std::make_pair((generic_func_t)start, (generic_func_t)stop);
                continue;
            }
        }

        if (iter + 1 != symbols_vector.end()) {
            printf("find complete!\nnext_name: %s\n", (iter+1)->second.c_str());
            dr_printf("%zu - %zu\n", iter->first, (iter+1)->first);
            res[func_name] = std::make_pair((generic_func_t)iter->first, (generic_func_t)(iter+1)->first);
        }
    }
    
    return res;
}

#endif // FIND_FUNC_BOUNDS_header
