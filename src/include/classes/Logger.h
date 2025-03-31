/**
 * @file Logger.h
 * @author Stepan Kafanov
 * @brief Logger class
 * @version 0.1
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef MY_LOGGER_header
#define MY_LOGGER_header

#include <string>
#include <fstream>
#include <filesystem>

#include "../funcs.h"

/**
 * @brief Logger class
 * 
 */
class Logger {
private:
    std::string log_file;
    std::ofstream stream;
public:
    Logger() {}

    Logger(const std::string & out_file) {
        this->log_file = out_file;
    }

    /**
     * @brief Set the file to logs collection
     * 
     * @param new_out_file - file to write logs in
     */
    void set_log_file(const std::string & new_out_file) {
        if (this->stream.is_open()) {
            this->stream.close();
        }
        this->log_file = new_out_file;
    }

    /**
     * @brief Prepare logger for working. Opens log-file descriptor
     * 
     * @param new_out_file - optional new log-file name to set
     * @param use_tid - shall logger log messages to file with thread-id prefix like '1234-log-file-name.txt'
     */
    void start_logging(std::string new_out_file = "", bool use_tid = false) {
        if (this->stream.is_open()) {
            this->stream.close();
        }
        if (!new_out_file.empty()) {
            this->log_file = new_out_file;
        }

        std::filesystem::path dir = std::filesystem::absolute(this->log_file).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }

        std::cout << "stream opening: " << this->log_file << std::endl;
        try {
            if (use_tid)
                this->stream.open(this->log_file + "-" + get_thread_id());
            else
                this->stream.open(this->log_file);
        } catch (...) {
            std::cout << "error with stream opening!" << std::endl;
        }
    }

    /**
     * @brief Close file descriptor / stream
     * 
     */
    void stop_logging() {
        if (this->stream.is_open()) {
            this->stream.close();
        } else {
            printf("logging is alreadry stopped!\n");
        }
    }

    /**
     * @brief arbitrary log function
     * @details apply args to '{}' inside passed message string and write it into log-file with passed `tag`
     * 
     * @tparam Args - args for 
     * @param tag - tag of message. message will be like "[tag] : message"
     * @param format - message string with '{}' to insert passed args
     * @param args - agrs to insert into format message string
     */
    template<typename... Args>
    void log(std::string tag, const std::string& format, Args... args) {
        if (!this->stream) {
            throw std::runtime_error("logging is impossible. log stream is closed!");
        }
        std::ostringstream oss;
        this->formatString(oss, format, args...);
        this->stream << "[" << tag << "] : " << oss.str() << std::endl;
    }
    template<typename T, typename... Args>
    void formatString(std::ostringstream& oss, const std::string& format, T value, Args... args) {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            oss << format.substr(0, pos) << value;
            formatString(oss, format.substr(pos + 2), args...);
        } else {
            oss << format;
        }
    }
    void formatString(std::ostringstream& oss, const std::string& format) {
        oss << format;
    }

    /// @brief checks whether logging stream is opened or not
    bool is_open() const {
        return this->stream.is_open();
    }


    /// @brief Logs info about client program and passed args: path, number of arguments (argc), arguments from argv
    /// @return 
    bool log_program_params() {
        this->log_line();
        this->stream << "[DEBUG] : " << "app_name: " << dr_get_application_name() << std::endl;
        int num_args = dr_num_app_args();
        this->stream << "[DEBUG] : " << "num_args: " << num_args << std::endl;

        if (num_args > 100) {
            // num of args must be less or equal to 100
            return false;
        }

        dr_app_arg_t args_array[100];
        int err = dr_get_app_args(args_array, num_args);
        if (err == -1) {
            this->stream << "[ERROR] : " << "cannot get app args" << std::endl;
            return false;
        }

        char buff[1000];
        for (int i = 0; i < num_args; ++i) {
            auto arg = dr_app_arg_as_cstring(&(args_array[i]), buff, sizeof(dr_app_arg_t)*10);
            this->stream << "[DEBUG] :\t" << "arg-" << i << ": " << arg << std::endl;
        }
        this->log_line();

        return true;
    }

    void log_line() {
        this->stream << "================================================================" << std::endl;
    }

    /// @brief Call this->log method with `tag = "DEBUG"`
    template<typename... Args>
    void log_debug(const std::string& format, Args... args) {
        this->log("DEBUG", format, args...);
    }
    /// @brief Call this->log method with `tag = "INFO"`
    template<typename... Args>
    void log_info(const std::string& format, Args... args) {
        this->log("INFO", format, args...);
    }
    /// @brief Call this->log method with `tag = "ERROR"`
    template<typename... Args>
    void log_error(const std::string& format, Args... args) {
        this->log("ERROR", format, args...);
    }
    /// @brief Call this->log method with `tag = "WARNING"`
    template<typename... Args>
    void log_warning(const std::string& format, Args... args) {
        this->log("WARNING", format, args...);
    }

    /// @brief Logs all visible by DR modules
    void log_modules() {
        auto modules = get_all_modules();
        this->stream << "[DEBUG] : modules:" << std::endl;
        for (auto module : modules) {
            this->stream << "\tmodule_name: " << module.name << "; module_path: " << module.path << std::endl;
        }
    }
};

#endif // MY_LOGGER_header