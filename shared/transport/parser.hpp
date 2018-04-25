#ifndef _transport_parser_hpp
#define _transport_parser_hpp

#pragma once

#include "transaction.hpp"

#include <cstddef>
#include <utility>


enum class parse_result
{
    ok, more, error
};

struct parse_context
{
    std::size_t state;
    std::size_t data;
    transaction txn;
};

class transaction_parser
{
public:
    transaction_parser() = default;

    template <typename Iterator>
    std::pair<parse_result, Iterator> operator()(parse_context& context, Iterator begin, Iterator end)
    {
        parse_result result = parse_result::more;
 
        while(begin != end)
        {
            result = consume(context, *begin++);

            if(result != parse_result::more)
            {
                break;
            }
        }

        return {result, begin};
    }

private:
    static parse_result consume(parse_context& context, char c);

private:
    enum
    {
        state_source,
        state_target,
        state_amount,
        state_amount1,
        state_currency,
        state_number
    };
};

#endif
