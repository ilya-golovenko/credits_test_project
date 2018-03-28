#ifndef _network_common_hpp
#define _network_common_hpp


namespace tcp::detail
{

void set_socket_option(int socket_fd, int level, int option, int value);

void set_socket_non_blocking(int socket_fd);

}   // namespace tcp::detail

#endif
