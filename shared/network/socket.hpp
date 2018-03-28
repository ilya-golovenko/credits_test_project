#ifndef _network_socket_hpp
#define _network_socket_hpp

#pragma once

#include "buffer.hpp"


namespace tcp
{

class socket
{
public:
    socket(int fd);

    socket();
    ~socket();

    socket(socket&& other);
    socket& operator=(socket&& other);

    socket(socket const&) = delete;
    socket& operator=(socket const&) = delete;

    void swap(socket& other);

    std::size_t write(const_buffer& buffer);
    std::size_t read(mutable_buffer& buffer);

    bool valid() const;

    void close();

    operator int() const;

private:
    int socket_fd_;
};

}   // namespace tcp

#endif
