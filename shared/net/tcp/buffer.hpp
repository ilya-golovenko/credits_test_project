#ifndef _net_buffer_hpp
#define _net_buffer_hpp

#pragma once

#include <cstddef>
#include <array>


namespace net
{

class const_buffer
{
public:
    const_buffer();

    const_buffer(void const* data, std::size_t size);

    const_buffer(const_buffer const&) = default;
    const_buffer& operator=(const_buffer const&) = default;

    void operator+=(std::size_t size);

    void const* data() const;
    std::size_t size() const;

    bool empty() const;

private:
    void const* data_;
    std::size_t size_;
};

class mutable_buffer
{
public:
    mutable_buffer();

    mutable_buffer(void* data, std::size_t size);

    mutable_buffer(mutable_buffer const&) = default;
    mutable_buffer& operator=(mutable_buffer const&) = default;

    void operator+=(std::size_t size);

    void* data() const;
    std::size_t size() const;

    bool empty() const;

private:
    void*       data_;
    std::size_t size_;
};

template <size_t N>
const_buffer buffer(std::array<char, N> const& data)
{
    return const_buffer(data.data(), data.size());
}

template <size_t N>
mutable_buffer buffer(std::array<char, N>& data)
{
    return mutable_buffer(data.data(), data.size());
}

}   // namespace net

#endif
