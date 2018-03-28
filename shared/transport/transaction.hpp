#ifndef _transport_transaction_hpp
#define _transport_transaction_hpp

#pragma once

#include <cstdint>
#include <iosfwd>


struct transaction
{
    char          source[10];
    char          target[10];
    std::uint32_t amount;
    std::uint64_t amount1;
    char          currency[10];
    int           number;
};

std::ostream& operator<<(std::ostream& os, transaction const& txn);

#endif
