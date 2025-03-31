/**
 * @file Options.h
 * @author Stepan Kafanov
 * @brief I adopted DynamoRIO parser for my project, because for some reason that parser didn't work
 * @version 0.1
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef MY_OPTIONS_header
#define MY_OPTIONS_header

#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <memory>

static bool
is_negative(const std::string &s)
{
    for (size_t i = 0; i < s.size(); i++) {
        if (isspace(s[i]))
            continue;
        if (s[i] == '-')
            return true;
        break;
    }
    return false;
}

class UnitypeOption {
public:
    std::string valsep_;
    std::string name_;
    bool is_specified_;
    std::string desc_short_;
    std::string desc_long_;
public:
    UnitypeOption(std::string name, std::string desc_short, std::string desc_long) 
        : name_(name)
        , desc_short_(desc_short)
        , desc_long_(desc_long)
        , valsep_(" ")
        , is_specified_(false) {}
    virtual ~UnitypeOption() {};

    std::string get_name() const {
        return name_;
    }
    bool name_match(const char *arg) {
        std::cout << "name matching" << std::endl;
        std::cout << this->name_ << " " << arg << std::endl;
        return std::string("-").append(this->name_) == arg || std::string("--").append(this->name_) == arg;
    }
    void set_is_specified(bool x) {
        is_specified_ = x;
    }
    bool is_specified() const {
        return is_specified_;
    }

    virtual std::string get_value_str() const = 0;
    virtual bool convert_from_string(const std::string s) = 0;
    virtual bool clamp_value() = 0;
    virtual bool option_takes_arg() const = 0;
    virtual void clear_value() {
        is_specified_ = false;
    }
};

template <typename T> 
class Option : public UnitypeOption {
public:
    T value_;
    T defval_;
    T minval_;
    T maxval_;
    bool has_range_;

    bool clamp_value() override {
        if (has_range_) {
            if (value_ < minval_) {
                value_ = minval_;
                return false;
            } else if (value_ > maxval_) {
                value_ = maxval_;
                return false;
            }
        }
        return true;
    }
public:
    Option(std::string name, T defval,                     std::string desc_short, std::string desc_long)
        : UnitypeOption(name, desc_short, desc_long)
        , value_(defval)
        , defval_(defval)
        , has_range_(false) {}
    Option(std::string name, T defval, T minval, T maxval, std::string desc_short, std::string desc_long)
        : UnitypeOption(name, desc_short, desc_long)
        , value_(defval)
        , defval_(defval)
        , has_range_(true)
        , minval_(minval)
        , maxval_(maxval) {}
    

    Option(const Option& option) 
        : UnitypeOption(option.name_, option.desc_short, option.desc_long)
        , value_(option.value_)
        , defval_(option.defval_)
        , has_range_(option.has_range_)
        , minval_(option.minval_)
        , maxval_(option.maxval_) {}
    Option& operator=(const Option& other)
    {
        if (this != &other) {
            value_ = other.value_;
            defval_ = other.defval_;
            valsep_ = other.valsep_;
            has_range_ = other.has_range_;
            minval_ = other.minval_;
            maxval_ = other.maxval_;
            name_ = other.name_;
            is_specified_ = other.is_specified_;
            desc_short_ = other.desc_short_;
            desc_long_ = other.desc_long_;
        }
        return *this;
    }
    ~Option() override = default;

    std::string get_value_str() const override {
        std::ostringstream ss;
        ss << value_;
        return ss.str();
    }

    std::string get_value_separator() const {
        return valsep_;
    }

    T get_value() const {
        return this->value_;
    }

    void clear_value() override {
        value_ = defval_;
        is_specified_ = false;
    }

    void set_value(T new_value) {
        value_ = new_value;
    }


    bool convert_from_string(const std::string s) override;
    bool option_takes_arg() const override;
};

class Parser {
public:
    std::vector <std::shared_ptr<UnitypeOption>> options_ = {};
public:
    Parser() {}

    bool parse_argv(int argc, const char *argv[], std::string *error_msg, int *last_index)
    {
        int i;
        bool res = true;
        for (i = 1 /*skip app*/; i < argc; ++i) {
            // We support the universal "--" as a separator
            if (strcmp(argv[i], "--") == 0) {
                ++i; // for last_index
                break;
            }
            // Also stop on a non-leading-dash token to support arguments without
            // a separating "--".
            if (argv[i][0] != '-') {
                break;
            }
            bool matched = false;
            for (int j = 0; j < this->options_.size(); ++j) {
                auto op = this->options_[j];
                if (op->name_match(argv[i])) {
                    matched = true;
                    std::cout << "option takes arg..." << std::endl;
                    if (op->option_takes_arg()) {
                        ++i;
                        if (i == argc) {
                            if (error_msg != NULL) {
                                std::cout << "error";
                                *error_msg = "Option " + op->get_name() + " missing value";
                            }
                            res = false;
                            goto parse_finished;
                        }
                        if (!op->convert_from_string(argv[i]) || !op->clamp_value()) {
                            if (error_msg != NULL) {
                                std::cout << "error";
                                *error_msg = "Option " + op->get_name() + " value out of range";
                            }
                            res = false;
                            goto parse_finished;
                        }
                    }
                    std::cout << "setting is specified..." << std::endl;
                    op->set_is_specified(true);
                    // op->is_specified_ = true; // *after* convert_from_string()
                }
            }
            if (!matched) {
                if (error_msg != NULL) {
                    std::cout << "error";
                    *error_msg = std::string("Unknown option: ") + argv[i];
                }
                res = false;
                goto parse_finished;
            }
        }
    parse_finished:
        if (last_index != NULL)
            *last_index = i;
        return res;
    }

    bool add_option(std::shared_ptr<UnitypeOption> option) {
        this->options_.push_back(option);
        return true;
    }

    std::vector <std::shared_ptr<UnitypeOption>> get_options() {
        return this->options_;
    }

    std::shared_ptr<UnitypeOption> & operator[](const std::string & name) {
        for (int i = 0; i < this->options_.size(); ++i) {
            if (this->options_[i]->get_name() == name) {
                return this->options_[i];
            }
        }
        throw std::runtime_error(std::string("there is not operator with such name: ") + name);
    }
    const std::shared_ptr<UnitypeOption> & operator[](const std::string & name) const {
        for (int i = 0; i < this->options_.size(); ++i) {
            if (this->options_[i]->get_name() == name) {
                return this->options_[i];
            }
        }
        throw std::runtime_error(std::string("there is not operator with such name: ") + name);
    }
};

template <>
inline bool
Option<std::string>::convert_from_string(const std::string s)
{
    std::cout << "casting string with <" << s << ">..." << std::endl;
    if (is_specified_)
        value_ += valsep_ + s;
    else
        value_ = s;
    return true;
}
template <>
inline bool
Option<int>::convert_from_string(const std::string s)
{
    std::cout << "casting int with <" << s << ">..." << std::endl;
    errno = 0;
    // If we set 0 as the base, strtol() will automatically identify the base of the
    // number to convert. By default, it will assume the number to be converted is
    // decimal, and a number starting with 0 or 0x is assumed to be octal or hexadecimal,
    // respectively.
    long input = strtol(s.c_str(), NULL, 0);
    std::cout << "res: " << input << std::endl;

    // strtol returns a long, but this may not always fit into an integer.
    if (input >= (long)INT_MIN && input <= (long)INT_MAX) {
        value_ = (int)input;
        std::cout << "success" << std::endl;
    } else
        return false;

    return errno == 0;
}
template <>
inline bool
Option<long>::convert_from_string(const std::string s)
{
    errno = 0;
    value_ = strtol(s.c_str(), NULL, 0);
    return errno == 0;
}
template <>
inline bool
Option<long long>::convert_from_string(const std::string s)
{
    errno = 0;
    // If we set 0 as the base, strtoll() will automatically identify the base.
    value_ = strtoll(s.c_str(), NULL, 0);
    return errno == 0;
}
template <>
inline bool
Option<unsigned int>::convert_from_string(const std::string s)
{
    errno = 0;
    long input = strtol(s.c_str(), NULL, 0);
    // Is the value positive and fits into an unsigned integer?
    if (input >= 0 && (unsigned long)input <= (unsigned long)UINT_MAX)
        value_ = (unsigned int)input;
    else
        return false;

    return errno == 0;
}
template <>
inline bool
Option<unsigned long>::convert_from_string(const std::string s)
{
    if (is_negative(s))
        return false;

    errno = 0;
    value_ = strtoul(s.c_str(), NULL, 0);
    return errno == 0;
}
template <>
inline bool
Option<unsigned long long>::convert_from_string(const std::string s)
{
    if (is_negative(s))
        return false;

    errno = 0;
    value_ = strtoull(s.c_str(), NULL, 0);
    return errno == 0;
}
template <>
inline bool
Option<double>::convert_from_string(const std::string s)
{
    // strtod will return 0.0 for invalid conversions
    char *pEnd = NULL;
    value_ = strtod(s.c_str(), &pEnd);
    return true;
}


template <typename T>
inline bool
Option<T>::option_takes_arg() const
{
    return true;
}
template <>
inline bool
Option<bool>::option_takes_arg() const
{
    return false;
}


#endif // MY_OPTIONS_header