#include "common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <system_error>


void tcp::detail::set_socket_option(int socket_fd, int level, int option, int value)
{
    if(::setsockopt(socket_fd, level, option, &value, sizeof(value)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "setsockopt failed");
    }
}

void tcp::detail::set_socket_non_blocking(int socket_fd)
{
    int flags = ::fcntl(socket_fd, F_GETFL, 0);

    if(flags < 0)
    {
        throw std::system_error(errno, std::system_category(), "fcntl failed");
    }

    if(::fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        throw std::system_error(errno, std::system_category(), "fcntl failed");
    }
}
