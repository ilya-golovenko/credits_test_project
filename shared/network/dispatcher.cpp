#include "dispatcher.hpp"

#include <log/logger.hpp>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <system_error>


tcp::dispatcher::dispatcher() :
    running_(false)
{
    if((kqueue_ = ::kqueue()) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kqueue");
    }
}

tcp::dispatcher::~dispatcher()
{
    stop();
}

void tcp::dispatcher::start()
{
    if(!running_.exchange(true))
    {
        worker_ = std::async([this]{ run(); });
    }
}

void tcp::dispatcher::stop()
{
    if(running_.exchange(false))
    {
        kqueue_.close();
        worker_.wait();
    }
}

void tcp::dispatcher::want_write(socket const& socket, ready_handler&& handler)
{
    operation op(socket, EVFILT_WRITE);

    // lock scope
    {
        std::lock_guard<std::mutex> lock(mutex_);

        handlers_.emplace(op, std::move(handler));
    }

    register_operation(op);
}

void tcp::dispatcher::want_read(socket const& socket, ready_handler&& handler)
{
    operation op(socket, EVFILT_READ);

    // lock scope
    {
        std::lock_guard<std::mutex> lock(mutex_);

        handlers_.emplace(op, std::move(handler));
    }

    register_operation(op);
}

void tcp::dispatcher::cancel_writes(socket const& socket)
{
    std::lock_guard<std::mutex> lock(mutex_);

    operation op(socket, EVFILT_WRITE);

    handlers_.erase(op);
}

void tcp::dispatcher::cancel_reads(socket const& socket)
{
    std::lock_guard<std::mutex> lock(mutex_);

    operation op(socket, EVFILT_READ);

    handlers_.erase(op);
}

void tcp::dispatcher::run()
{
    try
    {
        struct kevent events[32];

        for(;;)
        {
            int n = ::kevent(kqueue_, nullptr, 0, events, 32, nullptr);

            if(n < 0)
            {
                if(running_)
                {
                    throw std::system_error(errno, std::system_category(), "kevent");
                }

                break;
            }

            for(int i = 0; i < n; ++i)
            {
                std::error_code error;

                if(events[i].flags & EV_ERROR)
                {
                    error = std::error_code(events[i].data, std::system_category());
                }

                operation op(events[i].ident, events[i].filter);

                bool more = call_handler(op, error);

                if(more)
                {
                    register_operation(op);
                }
            }
        }
    }
    catch(std::system_error const& e)
    {
        log::error("caught std::system_error: ", e.what());
    }
    catch(std::exception const& e)
    {
        log::error("caught std::exception: ", e.what());
    }
    catch(...)
    {
        log::error("caught unexpected exception");
    }
}

void tcp::dispatcher::register_operation(operation const& op)
{
    struct kevent event;

    EV_SET(&event, op.first, op.second, EV_ADD | EV_ONESHOT, 0, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kevent");
    }
}

bool tcp::dispatcher::call_handler(operation const& op, std::error_code const& error)
{
    std::unique_lock<std::mutex> lock(mutex_);

    auto [begin, end] = handlers_.equal_range(op);

    if(begin == handlers_.end())
    {
        throw std::logic_error("operation handler is not found");
    }

    ready_handler handler(std::move(begin->second));

    handlers_.erase(begin++);

    bool more = begin != end;

    lock.unlock();

    handler(error);

    return more;
}
