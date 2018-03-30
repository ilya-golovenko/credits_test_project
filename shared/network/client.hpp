#ifndef _network_client_hpp
#define _network_client_hpp

#pragma once

#include "dispatcher.hpp"
#include "session.hpp"

#include <system_error>
#include <functional>
#include <cstdint>
#include <string>


namespace tcp
{

class client
{
public:
    using connect_handler = std::function<void (std::error_code const&)>;

public:
    explicit client(dispatcher& dispatcher);
    ~client();

    client(client const&) = delete;
    client& operator=(client const&) = delete;

    session& get_session();

    void connect(std::string const& server, std::uint16_t port, connect_handler&& handler);
    void close();

private:
    void do_connect(std::error_code const& error, connect_handler const& handler);

private:
    dispatcher& dispatcher_;
    session     session_;
};

}   // namespace tcp

#endif
