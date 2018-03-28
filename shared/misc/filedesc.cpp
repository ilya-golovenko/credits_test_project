#include "filedesc.hpp"

#include <utility>

#include <unistd.h>


filedesc::filedesc(int fd) :
    fd_(fd)
{}

filedesc::filedesc() :
    fd_(-1)
{}

filedesc::~filedesc()
{
    close();
}

filedesc::filedesc(filedesc&& other) :
    fd_(-1)
{
    swap(other);
}

filedesc& filedesc::operator=(filedesc&& other)
{
    swap(other);
    return *this;
}

void filedesc::swap(filedesc& other)
{
    std::swap(fd_, other.fd_);
}

void filedesc::close()
{
    if(fd_ >= 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
}

filedesc::operator int() const
{
    return fd_;
}
