#include "server.hpp"

#include <logging/logger.hpp>


transport::server::server(net::tcp::server& server, transaction_handler handler):
    server_{server},
    handler_{handler}
{}

void transport::server::listen(std::string const& address, std::uint16_t port)
{
    server_.listen(address, port);

    begin_accept();
}

void transport::server::begin_accept()
{
    server_.accept([this](auto&& session){ handle_accept(session); });
}

void transport::server::handle_accept(net::tcp::session& session)
{
    auto& [buffer, context] = sessions_[session];

    begin_read(session, net::tcp::buffer(buffer));

    begin_accept();
}

void transport::server::begin_read(net::tcp::session& session, net::tcp::mutable_buffer& buffer)
{
    session.read(buffer, [this, &session](auto&& error, std::size_t size){ handle_read(session, error, size); });
}

void transport::server::handle_read(net::tcp::session& session, std::error_code const& error, std::size_t size)
{
    if(!error)
    {
        if(size == 0)
        {
            return cleanup(session);
        }

        auto& [buffer, context] = sessions_.at(session);

        for(auto begin = buffer.begin(), end = begin + size; begin != end;)
        {
            auto [result, it] = parser_(context, begin, end);

            if(result == parse_result::error)
            {
                logging::error("cannot parse transaction: ", std::string(it, end));
                return cleanup(session);
            }

            if(result == parse_result::ok)
            {
                handler_(context.txn);
            }

            begin = it;
        }

        begin_read(session, net::tcp::buffer(buffer));
    }
    else
    {
        logging::error("failed to receive transactions: ", error.message());
        return cleanup(session);
    }
}

void transport::server::cleanup(net::tcp::session& session)
{
    sessions_.erase(session);
    session.close();
}
