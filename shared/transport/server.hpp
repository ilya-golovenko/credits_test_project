#ifndef _transport_server_hpp
#define _transport_server_hpp

#pragma once

#include "transaction.hpp"
#include "parser.hpp"

#include <network/server.hpp>

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
    server(tcp::server& server, transaction_handler handler);

    server(server const&) = delete;
    server& operator=(server const&) = delete;

    void listen(std::string const& address, std::uint16_t port);

private:
    void do_accept();
    void do_read(tcp::session& session);

    void handle_accept(tcp::session& session);
    void handle_read(tcp::session& session, std::error_code const& error, std::size_t size);

private:
    using read_buffer = std::array<char, 2048>;
    using buffer_map  = std::map<int, read_buffer>;
    using context_map = std::map<int, parse_context>;

private:
    tcp::server&        server_;
    transaction_handler handler_;
    transaction_parser  parser_;
    buffer_map          buffers_;
    context_map         contexts_;
};

}   // namespace transaction

#endif
