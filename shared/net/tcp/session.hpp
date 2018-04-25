#ifndef _net_tcp_session_hpp
#define _net_tcp_session_hpp

#pragma once

#include "socket.hpp"
#include "buffer.hpp"

#include <event/dispatcher.hpp>

#include <system_error>
#include <functional>
#include <utility>


namespace net::tcp
{

class session
{
public:
    using completion_handler = std::function<void (std::error_code const&, std::size_t)>;

public:
    session(socket&& socket, event::dispatcher& dispatcher);
    explicit session(event::dispatcher& dispatcher);
    ~session();

    session(session&&) = default;
    session& operator=(session&&) = default;

    session(session const&) = delete;
    session& operator=(session const&) = delete;

    socket& get_socket();

    void write(const_buffer_sequence const& buffers, completion_handler handler);
    void read(mutable_buffer_sequence const& buffers, completion_handler handler);

    void write(const_buffer const& buffer, completion_handler handler);
    void read(mutable_buffer const& buffer, completion_handler handler);

    void close();

    operator int() const;

private:
    struct write_operation
    {
        const_buffer_sequence buffers;
        completion_handler    handler;
        std::size_t           size;
    };

    struct read_operation
    {
        mutable_buffer_sequence buffers;
        completion_handler      handler;
        std::size_t             size;
    };

private:
    void do_write(std::error_code error, write_operation op);
    void do_read(std::error_code error, read_operation op);

private:
    socket             socket_;
    event::dispatcher& dispatcher_;
};

}   // namespace net::tcp

#endif
