#include "server.hpp"
#include "common.hpp"

#include <log/logger.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>


tcp::server::server(dispatcher& dispatcher) :
    dispatcher_(dispatcher)
{}

void tcp::server::listen(std::string const& address, std::uint16_t port)
{
    detail::set_socket_option(listener_, IPPROTO_TCP, TCP_NODELAY, 1);
    detail::set_socket_option(listener_, SOL_SOCKET, SO_KEEPALIVE, 1);
    detail::set_socket_option(listener_, SOL_SOCKET, SO_REUSEADDR, 1);

    union
    {
        struct sockaddr    sockaddr;
        struct sockaddr_in sockaddr_in;
    };

    sockaddr_in = {};

    sockaddr_in.sin_family      = AF_INET;
    sockaddr_in.sin_port        = htons(port);
    sockaddr_in.sin_addr.s_addr = inet_addr(address.c_str());

    if(::bind(listener_, &sockaddr, sizeof(sockaddr_in)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "bind");
    }

    if(::listen(listener_, SOMAXCONN) < 0)
    {
        throw std::system_error(errno, std::system_category(), "listen");
    }

    detail::set_socket_non_blocking(listener_);

    log::info("server is listening on ", address, ':', port);
}

void tcp::server::accept(accept_handler&& handler)
{
    dispatcher_.want_read(listener_, [this, handler = std::move(handler)](auto&& error){ do_accept(error, handler); });
}

void tcp::server::do_accept(std::error_code const& error, accept_handler const& handler)
{
    for(;;)
    {
        union
        {
            struct sockaddr    sockaddr;
            struct sockaddr_in sockaddr_in;
        };

        sockaddr_in = {};

        socklen_t socklen = sizeof(sockaddr_in);

        int socket_fd = ::accept(listener_, &sockaddr, &socklen);

        if(socket_fd < 0)
        {
            if(errno != EAGAIN && errno != EWOULDBLOCK)
            {
                throw std::system_error(errno, std::system_category(), "accept");
            }

            break;
        }

        socket socket(socket_fd);

        detail::set_socket_non_blocking(socket_fd);

        std::string const& host(detail::get_host_name(sockaddr, socklen));

        log::info("accepted connection from host ", host, " on socket ", socket_fd);

        sessions_.erase(socket_fd);
        sessions_.emplace(socket_fd, session(std::move(socket), dispatcher_));

        handler(std::ref(sessions_.at(socket_fd)));
    }
}

