#include "socket.hpp"

#include <log/logger.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <utility>


tcp::socket::socket(int fd) :
    socket_fd_(fd)
{
    if(socket_fd_ >= 0)
    {
        log::debug("socket[", socket_fd_, "] created");
    }
}

tcp::socket::socket()
{
    if((socket_fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket failed");
    }

    log::debug("socket[", socket_fd_, "] created");
}

tcp::socket::~socket()
{
    close();
}

tcp::socket::socket(socket&& other) :
    socket_fd_(-1)
{
    swap(other);
}

tcp::socket& tcp::socket::operator=(socket&& other)
{
    swap(other);
    return *this;
}

void tcp::socket::swap(socket& other)
{
    std::swap(socket_fd_, other.socket_fd_);
}

std::size_t tcp::socket::write(const_buffer& buffer)
{
    std::size_t sent = 0;

    while(!buffer.empty())
    {
        ssize_t count = ::write(socket_fd_, buffer.data(), buffer.size());

        if(count < 0)
        {
            if(errno != EAGAIN && errno != EWOULDBLOCK)
            {
                throw std::system_error(errno, std::system_category(), "write failed");
            }

            break;
        }

        buffer += count;
        sent   += count;
    }

    log::debug("socket[", socket_fd_, "] sent ", sent, " bytes");

    return sent;
}

std::size_t tcp::socket::read(mutable_buffer& buffer)
{
    std::size_t read = 0;

    while(!buffer.empty())
    {
        ssize_t count = ::read(socket_fd_, buffer.data(), buffer.size());

        if(count < 0)
        {
            // check if a non-blocking socket has no more data available
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }

            switch(errno)
            {
                case ETIMEDOUT:
                case EINTR:
                    continue;   // retry operation

                case ECONNRESET:
                case ENOTCONN:
                    count = 0;
                    break;      // connection closed

                default:
                    throw std::system_error(errno, std::system_category(), "read failed");
            }
        }

        if(count == 0)
        {
            log::debug("socket[", socket_fd_, "] peer closed connection");
            break;
        }

        buffer += count;
        read   += count;
    }

    if(read > 0)
    {
        log::debug("socket[", socket_fd_, "] received ", read, " bytes");
    }

    return read;
}

bool tcp::socket::valid() const
{
    return socket_fd_ >= 0;
}

void tcp::socket::close()
{
    if(socket_fd_ >= 0)
    {
        log::debug("socket[", socket_fd_, "] closed");

        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

tcp::socket::operator int() const
{
    return socket_fd_;
}
