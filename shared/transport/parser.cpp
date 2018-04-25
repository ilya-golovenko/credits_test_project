#include "parser.hpp"

#include <limits>
#include <locale>

#include <iostream>

namespace
{

template <char sep, size_t N>
parse_result consume_string_field(char (&field)[N], std::size_t& length, char c)
{
    if(c == sep)
    {
        if(length > 0)
        {
            field[length] = '\0';
            return parse_result::ok;
        }
    }
    else if(length < N - 1)
    {
        field[length++] = c;
        return parse_result::more;
    }

    return parse_result::error;
}

template <char sep, typename T>
parse_result consume_numeric_field(T& field, std::size_t& digits, char c)
{
    if(c == sep)
    {
        if(digits > 0)
        {
            return parse_result::ok;
        }
    }
    else if(std::isdigit(c, std::locale::classic()))
    {
        if(digits == 0)
        {
            ++digits;
            field = c - '0';
            return parse_result::more;
        }
        else if(digits <= std::numeric_limits<T>::digits10)
        {
            ++digits;
            field *= 10;
            field += c - '0';
            return parse_result::more;
        }
    }

    return parse_result::error;
}

}   // namespace

parse_result transaction_parser::consume(parse_context& context, char c)
{
    parse_result result = parse_result::error;

    switch(auto& [state, data, txn] = context; state)
    {
        case state_source:
            result = consume_string_field<'/'>(txn.source, data, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                state = state_target;
                data = 0;
            }
            break;

        case state_target:
            result = consume_string_field<'/'>(txn.target, data, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                state = state_amount;
                data = 0;
            }
            break;

        case state_amount:
            result = consume_numeric_field<':'>(txn.amount, data, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                state = state_amount1;
                data = 0;
            }
            break;

        case state_amount1:
            result = consume_numeric_field<'/'>(txn.amount1, data, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                state = state_currency;
                data = 0;
            }
            break;

        case state_currency:
            result = consume_string_field<'/'>(txn.currency, data, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                state = state_number;
                data = 0;
            }
            break;

        case state_number:
            result = consume_numeric_field<'/'>(txn.number, data, c);
            if(result == parse_result::ok)
            {
                state = state_source;
                data = 0;
            }
            break;
    }

    return result;
}
