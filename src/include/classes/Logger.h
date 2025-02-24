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
        std::filesystem::path dir = std::filesystem::absolute(this->log_file).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }

        this->stream.open(this->log_file);
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
};

#endif // MY_LOGGER_header