#ifndef _net_tcp_server_hpp
#define _net_tcp_server_hpp

#pragma once

#include "session.hpp"
#include "socket.hpp"

#include <event/dispatcher.hpp>

#include <system_error>
#include <functional>
#include <cstdint>
#include <string>
#include <map>


namespace net::tcp
{

class server
{
public:
    using accept_handler = std::function<void (session&)>;

public:
    explicit server(event::dispatcher& dispatcher);

    server(server const&) = delete;
    server& operator=(server const&) = delete;

    void listen(std::string const& address, std::uint16_t port);
    void accept(accept_handler handler);

private:
    void do_accept(std::error_code const& error, accept_handler const& handler);

private:
    using session_map = std::map<int, session>;

private:
    event::dispatcher& dispatcher_;
    session_map        sessions_;
    socket             listener_;
};

}   // namespace net::tcp

#endif
