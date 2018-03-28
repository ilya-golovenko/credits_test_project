#ifndef _network_buffer_hpp
#define _network_buffer_hpp

#pragma once

#include <cstddef>


namespace tcp
{

class const_buffer
{
public:
    const_buffer(void const* data, std::size_t size);

    const_buffer(const_buffer const& other) = default;
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
    mutable_buffer(void* data, std::size_t size);

    mutable_buffer(mutable_buffer const& other) = default;
    mutable_buffer& operator=(mutable_buffer const&) = default;

    void operator+=(std::size_t size);

    void* data() const;
    std::size_t size() const;

    bool empty() const;

private:
    void*       data_;
    std::size_t size_;
};

}   // namespace tcp

#endif
