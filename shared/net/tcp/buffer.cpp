#include "buffer.hpp"


tcp::const_buffer::const_buffer() :
    data_{nullptr},
    size_{0}
{}

tcp::const_buffer::const_buffer(void const* data, std::size_t size) :
    data_{data},
    size_{size}
{}

void tcp::const_buffer::operator+=(std::size_t size)
{
    std::size_t offset = size < size_ ? size : size_;
    data_ = static_cast<char const*>(data_) + offset;
    size_ -= offset;
}

void const* tcp::const_buffer::data() const
{
    return data_;
}

std::size_t tcp::const_buffer::size() const
{
    return size_;
}

bool tcp::const_buffer::empty() const
{
    return size_ == 0;
}

tcp::mutable_buffer::mutable_buffer() :
    data_{nullptr},
    size_{0}
{}

tcp::mutable_buffer::mutable_buffer(void* data, std::size_t size) :
    data_{data},
    size_{size}
{}

void tcp::mutable_buffer::operator+=(std::size_t size)
{
    std::size_t offset = size < size_ ? size : size_;
    data_ = static_cast<char*>(data_) + offset;
    size_ -= offset;
}

void* tcp::mutable_buffer::data() const
{
    return data_;
}

std::size_t tcp::mutable_buffer::size() const
{
    return size_;
}

bool tcp::mutable_buffer::empty() const
{
    return size_ == 0;
}
