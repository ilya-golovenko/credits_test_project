#include "buffer.hpp"

#include <cstdint>


tcp::const_buffer::const_buffer(void const* data, std::size_t size) :
    data_(data),
    size_(size)
{}

void tcp::const_buffer::operator+=(std::size_t size)
{
    data_ = static_cast<std::uint8_t const*>(data_) + size;
    size_ -= size;
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

tcp::mutable_buffer::mutable_buffer(void* data, std::size_t size) :
    data_(data),
    size_(size)
{}

void tcp::mutable_buffer::operator+=(std::size_t size)
{
    data_ = static_cast<std::uint8_t*>(data_) + size;
    size_ -= size;
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
