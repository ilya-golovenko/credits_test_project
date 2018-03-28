#include "parser.hpp"

#include <limits>
#include <locale>


namespace
{

template <char separator, size_t N>
parse_result consume_string_field(char (&field)[N], parse_context& context, char c)
{
    if(c == separator)
    {
        if(context.data > 0)
        {
            field[context.data] = '\0';
            return parse_result::ok;
        }
    }
    else if(context.data < N - 1)
    {
        field[context.data++] = c;
        return parse_result::more;
    }

    return parse_result::error;
}

template <char separator, typename T>
parse_result consume_numeric_field(T& field, parse_context& context, char c)
{
    if(c == separator)
    {
        if(context.data > 0)
        {
            return parse_result::ok;
        }
    }
    else if(std::isdigit(c, std::locale::classic()))
    {
        if(context.data++ == 0)
        {
            field = c - '0';
            return parse_result::more;
        }
        else if(context.data < std::numeric_limits<T>::digits10)
        {
            field = 10 * field + c - '0';
            return parse_result::more;
        }
    }

    return parse_result::error;
}

}   // namespace

parse_result transaction_parser::consume(parse_context& context, char c)
{
    parse_result result = parse_result::error;

    switch(context.state)
    {
        case state_source:
            result = consume_string_field<'|'>(context.txn.source, context, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                context.state = state_target;
                context.data = 0;
            }
            break;

        case state_target:
            result = consume_string_field<'|'>(context.txn.target, context, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                context.state = state_amount;
                context.data = 0;
            }
            break;

        case state_amount:
            result = consume_numeric_field<':'>(context.txn.amount, context, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                context.state = state_amount1;
                context.data = 0;
            }
            break;

        case state_amount1:
            result = consume_numeric_field<'|'>(context.txn.amount1, context, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                context.state = state_currency;
                context.data = 0;
            }
            break;

        case state_currency:
            result = consume_string_field<'|'>(context.txn.currency, context, c);
            if(result == parse_result::ok)
            {
                result = parse_result::more;
                context.state = state_number;
                context.data = 0;
            }
            break;

        case state_number:
            result = consume_numeric_field<'|'>(context.txn.number, context, c);
            if(result == parse_result::ok)
            {
                context.state = state_source;
                context.data = 0;
            }
            break;
    }

    return result;
}
