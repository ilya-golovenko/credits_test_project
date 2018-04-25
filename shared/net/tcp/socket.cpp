#include "socket.hpp"

#include <logging/logger.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


net::tcp::socket::socket(int fd) :
    fd_(fd)
{
    if(fd_.valid())
    {
        logging::debug("socket[", fd_ , "] created");
    }
}

net::tcp::socket::socket()
{
    if((fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    logging::debug("socket[", fd_ , "] created");
}

net::tcp::socket::~socket()
{
    close();
}

void net::tcp::socket::swap(socket& other) noexcept
{
    fd_.swap(other.fd_);
}

std::size_t net::tcp::socket::write(const_buffer_sequence const& buffers)
{
    std::size_t sent = 0;

    while(!buffers.empty())
    {
        ssize_t count = ::sendmsg(fd_, buffers.prepare(), MSG_NOSIGNAL);

        if(count < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }

            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }

            throw std::system_error(errno, std::system_category(), "sendmsg");
        }

        buffers += count;
        sent    += count;
    }

    logging::debug("socket[", fd_ , "] sent ", sent, " bytes");

    return sent;
}

std::size_t net::tcp::socket::read(mutable_buffer_sequence const& buffers)
{
    std::size_t read = 0;

    while(!buffers.empty())
    {
        ssize_t count = ::readmsg(fd_, buffers.prepare(), 0);

        if(count < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }

            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }

            if(errno == ENOTCONN || errno == ECONNRESET)
            {
                logging::debug("socket[", fd_ , "] peer closed connection");
                break;
            }

            throw std::system_error(errno, std::system_category(), "readmsg");
        }

        buffers += count;
        read    += count;
    }

    if(read > 0)
    {
        logging::debug("socket[", fd_ , "] received ", read, " bytes");
    }

    return read;
}

bool net::tcp::socket::valid() const
{
    return fd_.valid();
}

void net::tcp::socket::close()
{
    if(fd_.valid())
    {
        logging::debug("socket[", fd_ , "] closed");
        fd_.close();
    }
}

int net::tcp::socket::get() const
{
    return fd_.get();
}

net::tcp::socket::operator int() const
{
    return fd_.get();
}
