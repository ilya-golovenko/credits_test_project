#ifndef _tcp_buffer_sequence_hpp
#define _tcp_buffer_sequence_hpp

#pragma once

#include <cstddef>


namespace tcp
{

template <typename Buffer, size_t N = 64>
class buffer_sequence
{
public:
    buffer_sequence() :
        count_{0}
    {}

    buffer_sequence(buffer_sequence const&) = default;
    buffer_sequence& operator=(buffer_sequence const&) = default;

    bool can_append(std::size_t n) const
    {
        return (count_ + n) < N;
    }

    void append(Buffer const& buffer)
    {
        if(count_ < N)
        {
            buffers_[count_++] = buffer;
        }
    }

    std::size_t count() const
    {
        return count_;
    }

    bool all_empty() const
    {
        return false;
    }

    void operator+=(std::size_t size)
    {
        for(Buffer& buffer : buffers_)
        {
            if(size == 0)
            {
                break;
            }

            if(buffer.empty())
            {
                continue;
            }


        }
    }

private:
    std::array<Buffer, N> buffers_;
    std::size_t           count_;
};

}   // namespace tcp

template <>
class buffer_sequence<const_buffer, 1>
{
public:
    buffer_sequence(const_buffer const& buffer) :
        buffer_{buffer}
    {}

    buffer_sequence(buffer_sequence const&) = default;
    buffer_sequence& operator=(buffer_sequence const&) = default;

    std::size_t count() const
    {
        return 1;
    }

    bool all_empty() const
    {
        return buffer_.empty();
    }

    void operator+=(std::size_t size)
    {
        buffer_ += size;
    }

private:
    const_buffer buffer_;
};

template <>
class buffer_sequence<mutable_buffer, 1>
{
public:
    buffer_sequence(mutable_buffer const& buffer) :
        buffer_{buffer}
    {}

    buffer_sequence(buffer_sequence const&) = default;
    buffer_sequence& operator=(buffer_sequence const&) = default;

    std::size_t count() const
    {
        return 1;
    }

    bool all_empty() const
    {
        return buffer_.empty();
    }

    void operator+=(std::size_t size)
    {
        buffer_ += size;
    }

private:
    mutable_buffer buffer_;
};

#endif
