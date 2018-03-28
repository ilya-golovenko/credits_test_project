#ifndef _network_session_hpp
#define _network_session_hpp

#pragma once

#include "dispatcher.hpp"
#include "socket.hpp"
#include "buffer.hpp"

#include <system_error>
#include <functional>
#include <utility>
#include <queue>


namespace tcp
{

class session
{
public:
    using completion_handler = std::function<void (std::error_code const&, std::size_t)>;

public:
    session(socket&& socket, dispatcher& dispatcher);
    explicit session(dispatcher& dispatcher);
    ~session();

    session(session&&) = default;
    session& operator=(session&&) = default;

    session(session const&) = delete;
    session& operator=(session const&) = delete;

    socket& get_socket();

    void write(const_buffer const& buffer, completion_handler&& handler);
    void read(mutable_buffer const& buffer, completion_handler&& handler);

    void close();

private:
    void do_write();
    void do_read();

private:
    using write_queue = std::queue<std::pair<const_buffer, completion_handler>>;
    using read_queue  = std::queue<std::pair<mutable_buffer, completion_handler>>;

private:
    socket      socket_;
    dispatcher& dispatcher_;
    write_queue write_queue_;
    read_queue  read_queue_;
};

}   // namespace tcp

#endif
