#include "client.hpp"

#include <log/logger.hpp>

#include <sstream>


using namespace std::placeholders;

transport::client::client(tcp::client& client, transaction_handler handler):
    client_(client),
    handler_(handler)
{}

void transport::client::connect(std::string const& address, std::uint16_t port)
{
    log::info("connecting to server on ", address, ':', port);

    client_.connect(address, port, std::bind(&client::handle_connect, this, _1));
}

void transport::client::send(std::vector<transaction> const& transactions)
{
    log::info("sending ", transactions.size(), " transactions to server");

    std::ostringstream buffer;

    buffer.imbue(std::locale::classic());

    for(auto&& transaction : transactions)
    {
        buffer << transaction;
    }

    bool was_empty = write_queue_.empty();

    write_queue_.push(buffer.str());

    if(was_empty)
    {
        do_write(client_.get_session());
    }
}

void transport::client::do_write(tcp::session& session)
{
    std::string const& data(write_queue_.front());

    tcp::const_buffer buffer(data.data(), data.size());

    session.write(buffer, std::bind(&client::handle_write, this, _1, _2));
}

void transport::client::do_read(tcp::session& session)
{
    tcp::mutable_buffer buffer(buffer_.data(), buffer_.size());

    session.read(buffer, std::bind(&client::handle_read, this, _1, _2));
}

void transport::client::handle_connect(tcp::session& session)
{
    do_read(session);
}

void transport::client::handle_write(std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        write_queue_.pop();

        if(!write_queue_.empty())
        {
            do_write(client_.get_session());
        }
    }
    else
    {
        log::error("failed to send transactions: ", error.message());
    }
}

void transport::client::handle_read(std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        if(size == 0)
        {
            client_.close();
            return;
        }

        auto begin = buffer_.begin();
        auto end   = begin + size;

        while(begin != end)
        {
            parse_result result;

            std::tie(result, begin) = parser_(context_, begin, end);

            if(result == parse_result::error)
            {
                log::error("cannot parse received transaction: ", std::string(begin, end));
                client_.close();
            }
            else if(result == parse_result::ok)
            {
                if(handler_)
                {
                    handler_(context_.txn);
                }
            }
        }

        do_read(client_.get_session());
    }
    else
    {
        log::error("failed to read transactions: ", error.message());
        client_.close();
    }
}
