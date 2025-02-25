#ifndef MY_LOGGER_header
#define MY_LOGGER_header

#include <string>
#include <fstream>
#include <filesystem>

class Logger {
private:
    std::string log_file;
    std::ofstream stream;
public:
    Logger() {}

    Logger(const std::string & out_file) {
        this->log_file = out_file;
    }

    void set_log_file(const std::string & new_out_file) {
        if (this->stream.is_open()) {
            this->stream.close();
        }
        this->log_file = new_out_file;
    }

    void start_logging(std::string new_out_file = "") {
        if (this->stream.is_open()) {
            this->stream.close();
        }
        if (!new_out_file.empty()) {
            this->log_file = new_out_file;
        }

        // если дирректории с файлом нет - создаём её
        // std::filesystem::path dir = std::filesystem::absolute(this->log_file).parent_path();
        // if (!dir.empty() && !std::filesystem::exists(dir)) {
        //     std::filesystem::create_directories(dir);
        // }

        std::cout << "stream opening: " << this->log_file << std::endl;
        try {
            this->stream.open(this->log_file);
        } catch (...) {
            std::cout << "error with stream opening!" << std::endl;
        }
    }

    void stop_logging() {
        if (this->stream.is_open()) {
            this->stream.close();
        } else {
            printf("logging is alreadry stopped!\n");
        }
    }

    void log(std::string tag, std::string data) {
        if (!this->stream.is_open()) {
            printf("logging is impossible. log stream is closed!\n");
            throw std::runtime_error("logging is impossible. log stream is closed!");
        }
        this->stream << "[" << tag << "] : " << data << std::endl;
    }

    bool is_open() const {
        return this->stream.is_open();
    }


    bool log_program_params() {
        this->stream << "================================================================" << std::endl;
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
        this->stream << "================================================================" << std::endl;

        return true;
    }
};

#endif // MY_LOGGER_header