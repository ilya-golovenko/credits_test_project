#include "dispatcher.hpp"

#include <logging/logger.hpp>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>


static char const* get_filter_name(int filter)
{
    switch(filter)
    {
        case EVFILT_WRITE:
            return "write";

        case EVFILT_READ:
            return "read";

        case EVFILT_USER:
            return "user";
    }

    return "<unknown>";
}


event::dispatcher::dispatcher() :
    running_{true}
{
    if((kqueue_ = ::kqueue()) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kqueue");
    }

    register_event(0, EVFILT_USER, EV_CLEAR);

    thread_ = std::thread(&dispatcher::run, this);
}

event::dispatcher::~dispatcher()
{
    stop();
}

void event::dispatcher::stop()
{
    if(running_.exchange(false))
    {
        trigger_event(0, EVFILT_USER);

        thread_.wait();
        kqueue_.close();
    }
}

void event::dispatcher::want_write(int ident, ready_handler&& handler)
{
    if(add_event_handler(ident, EVFILT_WRITE, std::move(handler)))
    {
        register_event(ident, EVFILT_WRITE, EV_CLEAR);
    }
}

void event::dispatcher::want_read(int ident, ready_handler&& handler)
{
    if(add_event_handler(ident, EVFILT_READ, std::move(handler)))
    {
        register_event(ident, EVFILT_READ, EV_CLEAR);
    }
}

void event::dispatcher::post(ready_handler&& handler)
{
    if(add_event_handler(0, EVFILT_USER, std::move(handler)))
    {
        trigger_event(0, EVFILT_USER);
    }
}

void event::dispatcher::cancel_writes(int ident)
{
    if(remove_event_handler(ident, EVFILT_WRITE))
    {
        unregister_event(ident, EVFILT_WRITE);
    }
}

void event::dispatcher::cancel_reads(int ident)
{
    if(remove_event_handler(ident, EVFILT_READ))
    {
        unregister_event(ident, EVFILT_READ);
    }
}

void event::dispatcher::run()
{
    try
    {
        int const max_events = 32;

        struct kevent events[max_events];

        for(;;)
        {
            int n = ::kevent(kqueue_, nullptr, 0, events, max_events, nullptr);

            if(n < 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }

                throw std::system_error(errno, std::system_category(), "kevent");
            }

            if(!running_)
            {
                break;
            }

            for(int i = 0; i < n; ++i)
            {
                std::error_code error;

                if(events[i].flags & EV_ERROR)
                {
                    error.assign(events[i].data, std::system_category());
                }

                if(!call_event_handler(events[i].ident, events[i].filter, error))
                {
                    unregister_event(events[i].ident, events[i].filter);
                }
            }
        }
    }
    catch(std::system_error const& e)
    {
        logging::error("caught std::system_error: ", e.what());
    }
    catch(std::exception const& e)
    {
        logging::error("caught std::exception: ", e.what());
    }
    catch(...)
    {
        logging::error("caught unexpected exception");
    }
}

bool event::dispatcher::call_event_handler(int ident, int filter, std::error_code const& error)
{
    std::unique_lock<std::mutex> lock(mutex_);

    auto [it, end] = handlers_.equal_range({ident, filter});

    if(it != end)
    {
        ready_handler handler(std::move(it->second));

        it = handlers_.erase(it);

        lock.unlock();

        handler(error);

        return it != end;
    }

    return false;
}

bool event::dispatcher::add_event_handler(int ident, int filter, ready_handler&& handler)
{
    std::lock_guard<std::mutex> lock(mutex_);

    handler_map::iterator it = handlers_.upper_bound({ident, filter});

    handlers_.emplace_hint(it, {ident, filter}, std::move(handler));

    return it == std::end(handlers_);
}

bool event::dispatcher::remove_event_handler(int ident, int filter)
{
    std::lock_guard<std::mutex> lock(mutex_);

    return handlers_.erase({ident, filter}) != 0;
}

void event::dispatcher::register_event(int ident, int filter, int flags)
{
    logging::debug("adding ", ::get_filter_name(filter), " filter for descriptor ", ident);

    struct kevent event;

    EV_SET(&event, ident, filter, EV_ADD | flags, 0, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kevent");
    }
}

void event::dispatcher::unregister_event(int ident, int filter)
{
    logging::debug("removing ", ::get_filter_name(filter), " filter for descriptor ", ident);

    struct kevent event;

    EV_SET(&event, ident, filter, EV_DELETE, 0, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        if(errno != ENOENT)
        {
            throw std::system_error(errno, std::system_category(), "kevent");
        }
    }
}

void event::dispatcher::trigger_event(int ident, int filter)
{
    struct kevent event;

    EV_SET(&event, ident, filter, 0, NOTE_TRIGGER, 0, nullptr);

    if(::kevent(kqueue_, &event, 1, nullptr, 0, nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "kevent");
    }
}
