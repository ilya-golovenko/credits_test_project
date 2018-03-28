#ifndef _log_logger_hpp
#define _log_logger_hpp

#pragma once

#include <iostream>
#include <fstream>
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
    record(severity severity, std::ostream& stream, std::mutex& mutex);
    ~record();

    record(record const&) = delete;
    record& operator=(record const&) = delete;

    template <typename T>
    std::ostream& operator<<(T const& value)
    {
        stream_ << value;
        return stream_;
    }

private:
    std::ostream&               stream_;
    std::lock_guard<std::mutex> lock_;
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
        (record(severity, file_, mutex_) << ... << args);
    }

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
