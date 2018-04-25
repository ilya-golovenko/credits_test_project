#ifndef _transport_server_hpp
#define _transport_server_hpp

#pragma once

#include "transaction.hpp"
#include "parser.hpp"

#include <net/tcp/server.hpp>

#include <system_error>
#include <functional>
#include <array>
#include <map>


namespace transport
{

class server
{
public:
    using transaction_handler = std::function<void (transaction const&)>;

public:
    server(net::tcp::server& server, transaction_handler handler);

    server(server const&) = delete;
    server& operator=(server const&) = delete;

    void listen(std::string const& address, std::uint16_t port);

private:
    void begin_accept();
    void handle_accept(tcp::session& session);

    void begin_read(net::tcp::session& session, net::tcp::mutable_buffer& buffer);
    void handle_read(net::tcp::session& session, std::error_code const& error, std::size_t size);

    void cleanup(net::tcp::session& session);

private:
    using read_buffer         = std::array<char, 4096>;
    using session_context     = std::pair<read_buffer, parse_context>;
    using session_context_map = std::map<int, session_context>;

private:
    net::tcp::server&   server_;
    transaction_handler handler_;
    session_context_map sessions_;
    transaction_parser  parser_;
};

}   // namespace transaction

#endif
