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

static std::ostream& operator<<(std::ostream& os, logging::severity severity)
{
    switch(severity)
    {
        case logging::severity::debug:
            os << "[DEBUG]";
            break;

        case logging::severity::info:
            os << "[INFO] ";
            break;

        case logging::severity::warn:
            os << "[WARN] ";
            break;

        case logging::severity::error:
            os << "[ERROR]";
            break;

        case logging::severity::fatal:
            os << "[FATAL]";
            break;
    }

    return os;
}

logging::record::record(severity severity)
{
    stream_.imbue(std::locale::classic());

    stream_ << std::chrono::system_clock::now() << ' '
            << std::this_thread::get_id() << ' '
            << severity << ' ';
}

std::string logging::record::str() const
{
    return stream_.str();
}

logging::file::file(std::string const& filename) :
    file_(filename, std::ios::app)
{
    if(file_.tellp() > 0)
    {
        file_ << '\n';
    }
}

void logging::file::write(record const& record)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if(file_)
    {
        file_ << record.str() << '\n';
    }
}

static logging::file& get_log_file_impl(std::string const* filename)
{
    static logging::file file(filename ? *filename : std::string{"default.log"});
    return file;
}

logging::file& logging::get_log_file()
{
    return get_log_file_impl(nullptr);
}

void logging::set_log_file(std::string const& filename)
{
    get_log_file_impl(&filename);
}
