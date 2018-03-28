#include "client.hpp"
#include "common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


tcp::client::client(dispatcher& dispatcher) :
    dispatcher_(dispatcher),
    session_(dispatcher)
{}

tcp::client::~client()
{
    close();
}

tcp::session& tcp::client::get_session()
{
    return session_;
}

void tcp::client::connect(std::string const& server, std::uint16_t port, connect_handler&& handler)
{
    socket& socket = session_.get_socket();

    detail::set_socket_option(socket, IPPROTO_TCP, TCP_NODELAY, 1);
    detail::set_socket_option(socket, SOL_SOCKET, SO_KEEPALIVE, 1);

    detail::set_socket_non_blocking(socket);

    union
    {
        struct sockaddr    sockaddr;
        struct sockaddr_in sockaddr_in;
    };

    sockaddr_in = {};

    sockaddr_in.sin_family      = AF_INET;
    sockaddr_in.sin_port        = htons(port);
    sockaddr_in.sin_addr.s_addr = inet_addr(server.c_str());

    if(::connect(socket, &sockaddr, sizeof(sockaddr_in)) < 0)
    {
        if(errno == EISCONN)
        {
            handler(session_);
        }
        else if(errno != EINPROGRESS)
        {
            throw std::system_error(errno, std::system_category(), "connect failed");
        }
        else
        {
            dispatcher_.want_write(socket, [this, handler = std::move(handler)]{ handler(session_); });
        }
    }
    else
    {
        handler(session_);
    }
}

void tcp::client::close()
{
    session_.close();
}
