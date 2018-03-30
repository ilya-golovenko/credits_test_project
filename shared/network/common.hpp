#ifndef _network_common_hpp
#define _network_common_hpp

#include <sys/types.h>
#include <sys/socket.h>

#include <string>


namespace tcp::detail
{

std::string get_host_name(struct sockaddr const& sockaddr, socklen_t socklen);

void set_socket_option(int socket_fd, int level, int option, int value);

void set_socket_non_blocking(int socket_fd);

}   // namespace tcp::detail

#endif
