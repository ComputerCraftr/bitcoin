// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BIGINTEGER_H
#define BITCOIN_BIGINTEGER_H

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

class CBigInteger
{
private:
    uint8_t *dataPtr = nullptr;
    uint16_t nBytes = 0;

public:
    CBigInteger(uint16_t bytes) :
    dataPtr((uint8_t*)malloc(bytes)),
    nBytes(bytes)
    {
        assert("Failed to allocate memory for CBigInteger!" && dataPtr);
    }

    CBigInteger(const CBigInteger& bigint) :
    dataPtr((uint8_t*)malloc(bigint.nBytes)),
    nBytes(bigint.nBytes)
    {
        assert("Failed to allocate memory for CBigInteger!" && dataPtr);
        if (bigint.dataPtr) {
            memcpy(dataPtr, bigint.dataPtr, bigint.nBytes);
        }
    }

    ~CBigInteger()
    {
        free(dataPtr);
    }
};

#endif // BITCOIN_BIGINTEGER_H
