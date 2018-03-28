#include "server.hpp"

#include <log/logger.hpp>


using namespace std::placeholders;

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
    server_.accept(std::bind(&server::handle_accept, this, _1));
}

void transport::server::do_read(tcp::session& session)
{
    read_buffer& read_buffer(buffers_[session.get_socket()]);

    tcp::mutable_buffer buffer(read_buffer.data(), read_buffer.size());

    session.read(buffer, std::bind(&server::handle_read, this, std::ref(session), _1, _2));
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
            session.close();
            return;
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
                session.close();
            }
            else if(result == parse_result::ok)
            {
                if(handler_)
                {
                    handler_(context.txn);
                }
            }
        }

        do_read(session);
    }
    else
    {
        log::error("failed to receive transactions: ", error.message());
        session.close();
    }
}

