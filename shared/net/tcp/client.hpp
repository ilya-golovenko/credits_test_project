#ifndef _net_tcp_client_hpp
#define _net_tcp_client_hpp

#pragma once

#include "session.hpp"

#include <event/dispatcher.hpp>

#include <system_error>
#include <functional>
#include <cstdint>
#include <string>
#include <atomic>


namespace net::tcp
{

class client
{
public:
    using connect_handler = std::function<void (std::error_code const&)>;

public:
    explicit client(event::dispatcher& dispatcher);
    ~client();

    client(client const&) = delete;
    client& operator=(client const&) = delete;

    session& get_session();

    void connect(std::string const& address, std::uint16_t port, connect_handler handler);
    void close();

    bool connected() const;

private:
    void handle_connect(std::error_code const& error, connect_handler const& handler);

private:
    event::dispatcher& dispatcher_;
    session            session_;
    std::atomic_bool   connected_;
};

}   // namespace net::tcp

#endif
