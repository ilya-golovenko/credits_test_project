#include "common.hpp"

#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <system_error>


std::string net::tcp::detail::get_host_name(struct sockaddr const& sockaddr, socklen_t socklen)
{
    char host[NI_MAXHOST] = {};

    if(::getnameinfo(&sockaddr, socklen, host, sizeof(host), nullptr, 0, NI_NUMERICHOST) < 0)
    {
        throw std::system_error(errno, std::system_category(), "getnameinfo");
    }

    return host;
}

void net::tcp::detail::set_socket_option(int socket_fd, int level, int option, int value)
{
    if(::setsockopt(socket_fd, level, option, &value, sizeof(value)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }
}

int net::tcp::detail::get_socket_option(int socket_fd, int level, int option)
{
    int value;

    if(::getsockopt(socket_fd, level, option, &value, sizeof(value)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "getsockopt");
    }

    return value;
}

void net::tcp::detail::set_socket_non_blocking(int socket_fd)
{
    int flags = ::fcntl(socket_fd, F_GETFL, 0);

    if(flags < 0)
    {
        throw std::system_error(errno, std::system_category(), "fcntl");
    }

    if(::fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        throw std::system_error(errno, std::system_category(), "fcntl");
    }
}

void net::tcp::detail::shutdown_socket(int socket_fd)
{
    if(::shutdown(socket_fd, SHUT_RDWR) < 0)
    {
        throw std::system_error(errno, std::system_category(), "shutdown");
    }
}
