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
        throw std::system_error(errno, std::system_category(), "kqueue failed");
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
    struct kevent event;

    EV_SET(&event, socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kevent failed");
    }

    std::lock_guard<std::mutex> lock(write_mutex_);

    write_handlers_.emplace(socket, std::move(handler));
}

void tcp::dispatcher::want_read(socket const& socket, ready_handler&& handler)
{
    struct kevent event;

    EV_SET(&event, socket, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kevent failed");
    }

    std::lock_guard<std::mutex> lock(read_mutex_);

    read_handlers_.emplace(socket, std::move(handler));
}

void tcp::dispatcher::cancel_write(socket const& socket)
{
    std::lock_guard<std::mutex> lock(write_mutex_);

    write_handlers_.erase(socket);
}

void tcp::dispatcher::cancel_read(socket const& socket)
{
    std::lock_guard<std::mutex> lock(read_mutex_);

    read_handlers_.erase(socket);
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
                    throw std::system_error(errno, std::system_category(), "kevent failed");
                }

                break;
            }

            for(int i = 0; i < n; ++i)
            {
                int socket_fd = events[i].ident;

                if(events[i].flags & EV_ERROR)
                {
                    cancel_write(socket_fd);
                    cancel_read(socket_fd);
                }
                else
                {
                    if(events[i].filter == EVFILT_WRITE)
                    {
                        call_write_handler(socket_fd);
                    }
                    else if(events[i].filter == EVFILT_READ)
                    {
                        call_read_handler(socket_fd);
                    }
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

void tcp::dispatcher::call_write_handler(int socket_fd)
{
    std::unique_lock<std::mutex> lock(write_mutex_);

    auto handler = write_handlers_.at(socket_fd);

    write_handlers_.erase(socket_fd);

    lock.unlock();

    handler();
}

void tcp::dispatcher::call_read_handler(int socket_fd)
{
    std::unique_lock<std::mutex> lock(read_mutex_);

    auto handler = read_handlers_.at(socket_fd);

    read_handlers_.erase(socket_fd);

    lock.unlock();

    handler();
}
