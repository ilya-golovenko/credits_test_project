#ifndef _transport_client_hpp
#define _transport_client_hpp

#pragma once

#include "parser.hpp"
#include "queue.hpp"

#include <net/tcp/client.hpp>

#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <array>
#include <queue>


namespace transport
{

class client
{
public:
    using transaction_handler = std::function<void (transaction const&)>;

public:
    client(net::tcp::client& client, transaction_handler handler);

    client(client const&) = delete;
    client& operator=(client const&) = delete;

    void connect(std::string const& address, std::uint16_t port);
    void send(std::vector<transaction> const& transactions);

private:
    void handle_connect(std::error_code const& error);

    void begin_write(net::tcp::session& session, net::tcp::const_buffer_sequence& buffers);
    void handle_write(std::error_code const& error, std::size_t size);

    void begin_read(net::tcp::session& session, net::tcp::mutable_buffer& buffer);
    void handle_read(std::error_code const& error, std::size_t size);

private:
    using read_buffer = std::array<char, 4096>;

private:
    net::tcp::client&   client_;
    transaction_handler handler_;
    transaction_queue<> transactions_;
    read_buffer         buffer_;
    transaction_parser  parser_;
    parse_context       context_;
};

}   // namespace transport

#endif
