#include "transaction.hpp"

#include <iostream>


std::ostream& operator<<(std::ostream& os, transaction const& txn)
{
    os << txn.source   << '/'
       << txn.target   << '/'
       << txn.amount   << ':'
       << txn.amount1  << '/'
       << txn.currency << '/'
       << txn.number   << '/';

    return os;
}
