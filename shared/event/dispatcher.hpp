#ifndef _event_dispatcher_hpp
#define _event_dispatcher_hpp

#pragma once

#include<misc/file_descriptor.hpp>

#include <system_error>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <map>


namespace event
{

class dispatcher
{
public:
    using ready_handler = std::function<void (std::error_code const&)>;

public:
    dispatcher();
    ~dispatcher();

    dispatcher(dispatcher const&) = delete;
    dispatcher& operator=(dispatcher const&) = delete;

    void stop();

    void want_write(int ident, ready_handler&& handler);
    void want_read(int ident, ready_handler&& handler);

    void post(ready_handler&& handler);

    void cancel_writes(int ident);
    void cancel_reads(int ident);

private:
    using handler_map = std::multimap<std::pair<int, int>, ready_handler>;

private:
    void run();

    bool call_event_handler(int ident, int filter, std::error_code const& error);
    bool add_event_handler(int ident, int filter, ready_handler&& hander);
    bool remove_event_handler(int ident, int filter);

    void register_event(int ident, int filter, int flags);
    void unregister_event(int ident, int filter);
    void trigger_event(int ident, int filter);

private:
    file_descriptor  kqueue_;
    handler_map      handlers_;
    std::mutex       mutex_;
    std::thread      thread_;
    std::atomic_bool running_;
};

}   // namespace event

#endif
