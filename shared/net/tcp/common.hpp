#ifndef _net_tcp_common_hpp
#define _net_tcp_common_hpp

#include <sys/types.h>
#include <sys/socket.h>

#include <string>


namespace net::tcp::detail
{

std::string get_host_name(struct sockaddr const& sockaddr, socklen_t socklen);

void set_socket_option(int socket_fd, int level, int option, int value);

int get_socket_option(int socket_fd, int level, int option);

void set_socket_non_blocking(int socket_fd);

void shutdown_socket(int socket_fd);

}   // namespace net::tcp::detail

#endif
