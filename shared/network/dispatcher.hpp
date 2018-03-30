#ifndef _network_dispatcher_hpp
#define _network_dispatcher_hpp

#pragma once

#include "socket.hpp"

#include<misc/file_descriptor.hpp>

#include <functional>
#include <atomic>
#include <future>
#include <mutex>
#include <map>


namespace tcp
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

    void start();
    void stop();

    void want_write(socket const& socket, ready_handler&& handler);
    void want_read(socket const& socket, ready_handler&& handler);

    void cancel_writes(socket const& socket);
    void cancel_reads(socket const& socket);

private:
    using operation   = std::pair<int, short>;
    using handler_map = std::multimap<operation, ready_handler>;

private:
    void run();

    void register_operation(operation const& op);

    bool call_handler(operation const& op, std::error_code const& error);

private:
    file_descriptor   kqueue_;
    handler_map       handlers_;
    std::mutex        mutex_;
    std::atomic_bool  running_;
    std::future<void> worker_;
};

}   // namespace tcp

#endif
