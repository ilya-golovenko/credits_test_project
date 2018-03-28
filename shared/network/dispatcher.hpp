#ifndef _network_dispatcher_hpp
#define _network_dispatcher_hpp

#pragma once

#include "socket.hpp"

#include<misc/filedesc.hpp>

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
    using ready_handler = std::function<void ()>;

public:
    dispatcher();
    ~dispatcher();

    dispatcher(dispatcher const&) = delete;
    dispatcher& operator=(dispatcher const&) = delete;

    void start();
    void stop();

    void want_write(socket const& socket, ready_handler&& handler);
    void want_read(socket const& socket, ready_handler&& handler);

    void cancel_ops(socket const& socket);

private:
    void run();

    void call_write_handler(int socket_fd);
    void call_read_handler(int socket_fd);

private:
    using handler_map = std::map<int, ready_handler>;

private:
    filedesc          kqueue_;
    handler_map       write_handlers_;
    handler_map       read_handlers_;
    std::mutex        write_mutex_;
    std::mutex        read_mutex_;
    std::atomic_bool  running_;
    std::future<void> worker_;
};

}   // namespace tcp

#endif
