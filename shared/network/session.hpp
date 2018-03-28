#ifndef _network_session_hpp
#define _network_session_hpp

#pragma once

#include "dispatcher.hpp"
#include "socket.hpp"
#include "buffer.hpp"

#include <system_error>
#include <functional>
#include <utility>


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
    void do_write(const_buffer buffer, completion_handler const& handler);
    void do_read(mutable_buffer buffer, completion_handler const& handler);

private:
    socket      socket_;
    dispatcher& dispatcher_;
};

}   // namespace tcp

#endif
