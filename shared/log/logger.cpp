#include "logger.hpp"

#include <stdexcept>
#include <locale>
#include <iomanip>
#include <thread>
#include <chrono>
#include <ctime>


static std::ostream& operator<<(std::ostream& os, std::chrono::system_clock::time_point time_point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);

    os << std::put_time(std::gmtime(&time), "%F %T");

    return os;
}

static std::ostream& operator<<(std::ostream& os, log::severity severity)
{
    switch(severity)
    {
        case log::severity::debug:
            os << "[DEBUG]";
            break;

        case log::severity::info:
            os << "[INFO] ";
            break;

        case log::severity::warn:
            os << "[WARN] ";
            break;

        case log::severity::error:
            os << "[ERROR]";
            break;

        case log::severity::fatal:
            os << "[FATAL]";
            break;
    }

    return os;
}

log::record::record(severity severity)
{
    stream_.imbue(std::locale::classic());

    stream_ << std::chrono::system_clock::now() << ' '
            << std::this_thread::get_id() << ' '
            << severity << ' ';
}

std::string log::record::str() const
{
    return stream_.str();
}

log::file::file(std::string const& filename) :
    file_(filename, std::ios::app)
{}

log::file::~file()
{
    if(file_)
    {
        file_ << std::endl;
    }
}

void log::file::write(record const& record)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if(file_)
    {
        file_ << record.str() << std::endl;
    }
    else
    {
        std::cout << record.str() << std::endl;
    }
}

static log::file& get_log_file_impl(std::string const* filename)
{
    static log::file file(filename ? *filename : std::string("default.log"));
    return file;
}

log::file& log::get_log_file()
{
    return get_log_file_impl(nullptr);
}

void log::set_log_file(std::string const& filename)
{
    get_log_file_impl(&filename);
}
