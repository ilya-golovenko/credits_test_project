#include "client.hpp"

#include <logging/logger.hpp>

#include <sstream>


transport::client::client(net::tcp::client& client, transaction_handler handler):
    client_{client},
    handler_{handler}
{}

void transport::client::connect(std::string const& address, std::uint16_t port)
{
    client_.connect(address, port, [this](auto&& error) { handle_connect(error); });
}

void transport::client::send(std::vector<transaction> const& transactions)
{
    if(!transactions.empty())
    {
        if(bool was_empty = transactions_.add(transactions); was_empty)
        {
            if(client_.connected())
            {
                begin_write(client_.get_session(), transactions_.prepare());
            }
        }
    }
}

void transport::client::handle_connect(std::error_code const& error)
{
    if(!error)
    {
        if(auto& buffers = transactions_.prepare(); !buffers.empty())
        {
            begin_write(client_.get_session(), buffers);
        }

        begin_read(client_.get_session(), net::tcp::buffer(buffer_));
    }
    else
    {
        logging::error("failed to connect to server: ", error.message());
    }
}

void transport::client::begin_write(net::tcp::session& session, net::tcp::const_buffer_sequence& buffers)
{
    session.write(buffers, [this](auto&& error, std::size_t size){ handle_write(error, size); });
}

void transport::client::handle_write(std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        if(auto& buffers = transactions_.prepare(); !buffers.empty())
        {
            begin_write(client_.get_session(), buffers);
        }
    }
    else
    {
        logging::error("failed to send transactions: ", error.message());
        return client_.close();
    }
}

void transport::client::begin_read(net::tcp::session& session, net::tcp::mutable_buffer& buffer)
{
    session.read(buffer, [this](auto&& error, std::size_t size){ handle_read(error, size); });
}

void transport::client::handle_read(std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        if(size == 0)
        {
            return client_.close();
        }

        for(auto begin = buffer.begin(), end = begin + size; begin != end;)
        {
            auto [result, it] = parser_(context_, begin, end);

            if(result == parse_result::error)
            {
                logging::error("cannot parse transaction: ", std::string(it, end));
                return client_.close();
            }

            if(result == parse_result::ok)
            {
                handler_(context_.txn);
            }

            begin = it;
        }

        begin_read(client_.get_session(), net::tcp::buffer(buffer_));
    }
    else
    {
        logging::error("failed to receive transactions: ", error.message());
        return client_.close();
    }
}
