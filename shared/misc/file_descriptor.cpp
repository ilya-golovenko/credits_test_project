#include "file_descriptor.hpp"

#include <utility>

#include <unistd.h>


file_descriptor::file_descriptor(int fd) :
    fd_{fd}
{}

file_descriptor::file_descriptor() :
    fd_{-1}
{}

file_descriptor::~file_descriptor()
{
    close();
}

file_descriptor::file_descriptor(file_descriptor&& other) :
    fd_{-1}
{
    swap(other);
}

file_descriptor& file_descriptor::operator=(file_descriptor&& other)
{
    swap(other);
    return *this;
}

void file_descriptor::swap(file_descriptor& other) noexcept
{
    std::swap(fd_, other.fd_);
}

bool file_descriptor::valid() const
{
    return fd_ >= 0;
}

void file_descriptor::close()
{
    if(fd_ >= 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
}

int file_descriptor::get() const
{
    return fd_;
}

file_descriptor::operator int() const
{
    return fd_;
}
