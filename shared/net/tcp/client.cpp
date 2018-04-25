#include "client.hpp"
#include "common.hpp"

#include <logging/logger.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


net::tcp::client::client(event::dispatcher& dispatcher) :
    dispatcher_(dispatcher),
    session_(dispatcher),
    connected_(false)
{}

net::tcp::client::~client()
{
    close();
}

net::tcp::session& net::tcp::client::get_session()
{
    return session_;
}

void net::tcp::client::connect(std::string const& address, std::uint16_t port, connect_handler handler)
{
    logging::info("connecting to server on ", address, ':', port);

    socket& socket = session_.get_socket();

    detail::set_socket_option(socket, IPPROTO_TCP, TCP_NODELAY, 1);
    detail::set_socket_option(socket, SOL_SOCKET, SO_KEEPALIVE, 1);
    detail::set_socket_option(socket, SOL_SOCKET, SO_NOSIGPIPE, 1);

    detail::set_socket_option(socket, SOL_SOCKET, SO_SNDBUF, 2'097'152);

    detail::set_socket_non_blocking(socket);

    union
    {
        struct sockaddr    sockaddr;
        struct sockaddr_in sockaddr_in;
    };

    sockaddr_in = {};

    sockaddr_in.sin_family      = AF_INET;
    sockaddr_in.sin_port        = htons(port);
    sockaddr_in.sin_addr.s_addr = inet_addr(address.c_str());

    std::error_code error;

    do
    {
        if(::connect(socket, &sockaddr, sizeof(sockaddr_in)) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }

            if(errno == EISCONN)
            {
                break;
            }

            if(errno == EINPROGRESS)
            {
                return dispatcher_.want_write(socket, [this, handler](auto&& error){ handle_connect(error, handler); });
            }

            error.assign(errno, std::system_category());
        }
    }
    while(false);

    dispatcher_.post([this, error, handler](auto){ handle_connect(error, handler); });
}

void net::tcp::client::close()
{
    connected_ = false;
    session_.close();
}

bool net::tcp::client::connected() const
{
    return connected_;
}

void net::tcp::client::handle_connect(std::error_code const& error, connect_handler const& handler)
{
    if(!error)
    {
        error.assign(detail::get_socket_option(session_.get_socket(), SOL_SOCKET, SO_ERROR), std::system_category());
    }

    if(!error)
    {
        connected_ = true;
    }

    handler(error);
}
