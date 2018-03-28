#ifndef _transport_client_hpp
#define _transport_client_hpp

#pragma once

#include "transaction.hpp"
#include "parser.hpp"

#include <network/client.hpp>

#include <functional>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <mutex>


namespace transport
{

class client
{
public:
    using transaction_handler = std::function<void (transaction const&)>;

public:
    client(tcp::client& client, transaction_handler handler);

    client(client const&) = delete;
    client& operator=(client const&) = delete;

    void connect(std::string const& address, std::uint16_t port);
    void send(std::vector<transaction> const& transactions);

private:
    void do_write(tcp::session& session);
    void do_read(tcp::session& session);

    void handle_connect(tcp::session& session);
    void handle_write(std::error_code const& error, std::size_t size);
    void handle_read(std::error_code const& error, std::size_t size);

private:
    using string_queue = std::queue<std::string>;
    using read_buffer  = std::array<char, 2048>;

private:
    tcp::client&        client_;
    read_buffer         buffer_;
    string_queue        write_queue_;
    std::mutex          write_mutex_;
    transaction_handler handler_;
    transaction_parser  parser_;
    parse_context       context_;
};

}   // namespace transport

#endif
