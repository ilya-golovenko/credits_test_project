#ifndef _net_tcp_socket_hpp
#define _net_tcp_socket_hpp

#pragma once

#include "buffer.hpp"

#include <misc/file_descriptor.hpp>

#include <cstddef>


namespace net::tcp
{

class socket
{
public:
    socket(int fd);

    socket();
    ~socket();

    socket(socket&&) = default;
    socket& operator=(socket&&) = default;

    socket(socket const&) = delete;
    socket& operator=(socket const&) = delete;

    void swap(socket& other) noexcept;

    std::size_t write(const_buffer_sequence const& buffers);
    std::size_t read(mutable_buffer_sequence const& buffers);

    bool valid() const;

    void close();

    int get() const;

    operator int() const;

private:
    file_descriptor fd_;
};

}   // namespace net::tcp

#endif
