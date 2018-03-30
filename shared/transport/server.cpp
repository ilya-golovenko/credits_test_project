#include "server.hpp"

#include <log/logger.hpp>


transport::server::server(tcp::server& server, transaction_handler handler):
    server_(server),
    handler_(handler)
{}

void transport::server::listen(std::string const& address, std::uint16_t port)
{
    server_.listen(address, port);

    do_accept();
}

void transport::server::do_accept()
{
    server_.accept([this](auto&& session){ handle_accept(session); });
}

void transport::server::do_read(tcp::session& session)
{
    read_buffer& read_buffer(buffers_[session.get_socket()]);

    tcp::mutable_buffer buffer(read_buffer.data(), read_buffer.size());

    session.read(buffer, [this, &session](auto&& error, std::size_t size){ handle_read(session, error, size); });
}

void transport::server::handle_accept(tcp::session& session)
{
    do_read(session);
    do_accept();
}

void transport::server::handle_read(tcp::session& session, std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        if(size == 0)
        {
            return cleanup(session);
        }

        tcp::socket& socket(session.get_socket());

        read_buffer&   buffer(buffers_[socket]);
        parse_context& context(contexts_[socket]);

        auto begin = buffer.begin();
        auto end   = begin + size;

        while(begin != end)
        {
            parse_result result;

            std::tie(result, begin) = parser_(context, begin, end);

            if(result == parse_result::error)
            {
                log::error("cannot parse transaction: ", std::string(begin, end));
                return cleanup(session);
            }

            if(result == parse_result::ok)
            {
                handler_(context.txn);
            }
        }

        do_read(session);
    }
    else
    {
        log::error("failed to receive transactions: ", error.message());
        return cleanup(session);
    }
}

void transport::server::cleanup(tcp::session& session)
{
    tcp::socket& socket(session.get_socket());

    buffers_.erase(socket);
    contexts_.erase(socket);

    session.close();
}
