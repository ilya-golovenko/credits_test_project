#ifndef _log_logger_hpp
#define _log_logger_hpp

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>


namespace log
{

enum class severity
{
    debug, info, warn, error, fatal
};

class record
{
public:
    explicit record(severity severity);

    record(record const&) = delete;
    record& operator=(record const&) = delete;

    template <typename T>
    record& operator<<(T const& value)
    {
        stream_ << value;
        return *this;
    }

    std::string str() const;

private:
    std::ostringstream stream_;
};

class file
{
public:
    explicit file(std::string const& filename);
    ~file();

    file(file&&) = delete;
    file& operator=(file&&) = delete;

    file(file const&) = delete;
    file& operator=(file const&) = delete;

    template <typename ... Args>
    void write(severity severity, Args const& ...args)
    {
        write((record(severity) << ... << args));
    }

private:
    void write(record const& record);

private:
    std::ofstream file_;
    std::mutex    mutex_;
};

void set_log_file(std::string const& filename);

file& get_log_file();

template <typename ... Args>
void debug(Args const& ...args)
{
    get_log_file().write(severity::debug, args...);
}

template <typename ... Args>
void info(Args const& ...args)
{
    get_log_file().write(severity::info, args...);
}

template <typename ... Args>
void warn(Args const& ...args)
{
    get_log_file().write(severity::warn, args...);
}

template <typename ... Args>
void error(Args const& ...args)
{
    get_log_file().write(severity::error, args...);
}

template <typename ... Args>
void fatal(Args const& ...args)
{
    get_log_file().write(severity::fatal, args...);
}

}   // namespace log

#endif
