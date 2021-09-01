// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BIGINTEGER_H
#define BITCOIN_BIGINTEGER_H

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdint>

class CBigInteger
{
private:
    uint8_t *dataPtr = nullptr;
    uint32_t nBytes = 0;

public:
    CBigInteger(uint32_t bytes)
    {
        nBytes = bytes;
        dataPtr = (uint8_t*)calloc(nBytes, 1);
        if (!IsInitialized()) {
            nBytes = 0;
        }
    }

    CBigInteger(const CBigInteger& bigint)
    {
        if (bigint.IsInitialized()) {
            nBytes = bigint.nBytes;
            dataPtr = (uint8_t*)malloc(nBytes);
            if (IsInitialized()) {
                memcpy(dataPtr, bigint.dataPtr, nBytes);
            } else {
                nBytes = 0;
            }
        }
    }

    CBigInteger(uint64_t num, uint32_t bytes)
    {
        nBytes = std::max(bytes, (uint32_t)sizeof(uint64_t));
        dataPtr = (uint8_t*)malloc(nBytes);
        if (IsInitialized()) {
            memcpy(dataPtr, &num, sizeof(uint64_t));
        } else {
            nBytes = 0;
        }

    }

    ~CBigInteger()
    {
        free(dataPtr);
    }

    void operator=(const CBigInteger& bigint)
    {
        if (bigint.IsInitialized()) {
            nBytes = bigint.nBytes;
            dataPtr = (uint8_t*)realloc(dataPtr, nBytes);
            if (IsInitialized()) {
                memcpy(dataPtr, bigint.dataPtr, nBytes);
            } else {
                nBytes = 0;
            }
        } else {
            free(dataPtr);
            dataPtr = nullptr;
            nBytes = 0;
        }
    }

    void operator=(uint64_t num)
    {
        if (nBytes >= sizeof(uint64_t)) {
            memset(dataPtr, '\0', nBytes);
        } else {
            nBytes = sizeof(uint64_t);
            dataPtr = (uint8_t*)realloc(dataPtr, nBytes);
        }

        if (IsInitialized()) {
            memcpy(dataPtr, &num, sizeof(uint64_t));
        } else {
            nBytes = 0;
        }
    }

    bool operator==(const CBigInteger& bigint)
    {
        return nBytes == bigint.nBytes && memcmp(dataPtr, bigint.dataPtr, nBytes) == 0;
    }

    bool IsInitialized() const
    {
        return dataPtr != nullptr && nBytes != 0;
    }

    uint64_t GetLow64() const
    {
        uint64_t ret = 0;
        memcpy(&ret, dataPtr, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    uint32_t LengthBytes() const
    {
        return nBytes;
    }

    uint32_t LengthBits() const
    {
        return nBytes * 8;
    }
};

#endif // BITCOIN_BIGINTEGER_H
