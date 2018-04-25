#ifndef _transport_queue_hpp
#define _transport_queue_hpp

# pragma once

#include "transaction.hpp"

#include <condition_variable>
#include <mutex>
#include <iterator>
#include <vector>
#include <deque>
#include <array>


namespace
{

template <char separator, size_t N>
net::const_buffer to_buffer(char (&field)[N])
{
    std::size_t length = std::strlen(field);

    field[length++] = separator;

    return {field, length};
}

template <char separator, typename T, typename Iterator>
net::const_buffer to_buffer(T field, Iterator output)
{
    Iterator start = output;

    *output++ = separator;

    do
    {
        *output++ = (field % 10) + '0';
    }
    while((field /= 10) > 0);

    auto data = std::addressof(*output);
    auto size = std::distance(start, output);

    return {data, size};
}

}   // namespace

template <size_t N = 1000>
class transaction_queue
{
public:
    transaction_queue() :
        prepared_num_{0}
    {}

    transaction_queue(transaction_queue const&) = delete;
    transaction_queue& operator=(transaction_queue const&) = delete;

    bool add(std::vector<transaction> const& transactions)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        bool was_empty = transactions_.empty();

        for(transaction const& transaction : transactions)
        {
            not_full_.wait(lock, [this]{ return transactions_.size() < N; });

            transactions_.push_back(transaction);
        }

        return was_empty;
    }

    tcp::const_buffers_iterator prepare()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if(prepared_num_ > 0)
        {
            transactions_.erase(transactions_.begin(), transactions_.begin() + prepared_num_);
        }

        auto tcp_iterator  = tcp_buffers_.begin();
        auto temp_iterator = temp_buffer_.rbegin();

        for(transaction& transaction : transactions_)
        {
            if(tcp_iterator + 6 < tcp_buffers_.end())
            {
                *tcp_iterator++ = to_buffer<'|'>(transaction.source);
                *tcp_iterator++ = to_buffer<'|'>(transaction.target);
                *tcp_iterator++ = to_buffer<':'>(transaction.amount, temp_iterator);
                *tcp_iterator++ = to_buffer<'|'>(transaction.amount1, temp_iterator);
                *tcp_iterator++ = to_buffer<'|'>(transaction.currency);
                *tcp_iterator++ = to_buffer<'|'>(transaction.number, temp_iterator);

                ++prepared_num_;
            }
            else
            {
                break;
            }
        }

        lock.unlock();
        not_full_.notify_one();

        return tcp_buffers_;
    }

private:
    using temp_buffer = std::array<char, 16384>;
    using tcp_buffers = std::array<net::const_buffer, 64>;

private:
    std::mutex              mutex_;
    std::condition_variable not_full_;
    std::deque<transaction> transactions_;
    std::size_t             prepared_num_;
    tcp_buffers             tcp_buffers_;
    temp_buffer             temp_buffer_;
};

#endif
