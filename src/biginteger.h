// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BIGINTEGER_H
#define BITCOIN_BIGINTEGER_H

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>

/*inline*/ constexpr char HEX_CHARS[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

class CBigInteger
{
private:
    uint8_t* dataPtr = nullptr;
    uint32_t nBytes = 0;

public:
    CBigInteger(const uint32_t& bytes, const bool zero)
    {
        nBytes = bytes;
        if (zero) {
            dataPtr = (uint8_t*)calloc(nBytes, 1);
        } else {
            dataPtr = (uint8_t*)malloc(nBytes);
        }
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

    CBigInteger(const uint64_t& num)
    {
        nBytes = sizeof(uint64_t);
        dataPtr = (uint8_t*)malloc(sizeof(uint64_t));
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
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            nBytes = bigint.nBytes;
            if (IsInitialized()) {
                memcpy(dataPtr, bigint.dataPtr, nBytes);
            } else {
                nBytes = 0;
            }
        } else {
            SetNull();
        }
    }

    void operator^=(const CBigInteger& bigint)
    {
        uint32_t bytes;
        if (nBytes < bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            memset(dataPtr + nBytes, '\0', bigint.nBytes - nBytes);
            nBytes = bigint.nBytes;
            bytes = nBytes;
        } else {
            bytes = bigint.nBytes;
        }

        for (uint32_t i = 0; i < bytes; i++) {
            dataPtr[i] ^= bigint.dataPtr[i];
        }
    }

    void operator&=(const CBigInteger& bigint)
    {
        const uint32_t bytes = std::min(nBytes, bigint.nBytes);

        for (uint32_t i = 0; i < bytes; i++) {
            dataPtr[i] &= bigint.dataPtr[i];
        }
    }

    void operator|=(const CBigInteger& bigint)
    {
        uint32_t bytes;
        if (nBytes < bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            memset(dataPtr + nBytes, '\0', bigint.nBytes - nBytes);
            nBytes = bigint.nBytes;
            bytes = nBytes;
        } else {
            bytes = bigint.nBytes;
        }

        for (uint32_t i = 0; i < bytes; i++) {
            dataPtr[i] |= bigint.dataPtr[i];
        }
    }

    void operator=(const uint64_t& num)
    {
        if (nBytes > sizeof(uint64_t)) {
            memset(dataPtr, '\0', nBytes);
        } else if (nBytes < sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            nBytes = sizeof(uint64_t);
        }

        if (IsInitialized()) {
            memcpy(dataPtr, &num, sizeof(uint64_t));
        } else {
            nBytes = 0;
        }
    }

    void operator^=(const uint64_t& num)
    {
        if (nBytes < sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            memset(dataPtr + nBytes, '\0', sizeof(uint64_t) - nBytes);
            nBytes = sizeof(uint64_t);
        }

        ((uint64_t*)dataPtr)[0] ^= num;
    }

    void operator&=(const uint64_t& num)
    {
        if (nBytes < sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            memset(dataPtr + nBytes, '\0', sizeof(uint64_t) - nBytes);
            nBytes = sizeof(uint64_t);
        }

        ((uint64_t*)dataPtr)[0] &= num;
    }

    void operator|=(const uint64_t& num)
    {
        if (nBytes < sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            memset(dataPtr + nBytes, '\0', sizeof(uint64_t) - nBytes);
            nBytes = sizeof(uint64_t);
        }

        ((uint64_t*)dataPtr)[0] |= num;
    }

    const CBigInteger operator~() const
    {
        CBigInteger ret(*this);
        for (uint32_t i = 0; i < ret.nBytes; i++) {
            ret.dataPtr[i] = ~ret.dataPtr[i];
        }
        return ret;
    }

    bool operator==(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) == 0;
    }

    bool operator!=(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) != 0;
    }

    bool operator>(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) > 0;
    }

    bool operator<(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) < 0;
    }

    bool operator>=(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) >= 0;
    }

    bool operator<=(const CBigInteger& bigint) const
    {
        if (nBytes > bigint.nBytes) {
            for (uint32_t i = bigint.nBytes; i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        } else if (nBytes < bigint.nBytes) {
            for (uint32_t i = nBytes; i < bigint.nBytes; i++) {
                if (bigint.dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return memcmp(dataPtr, bigint.dataPtr, std::min(nBytes, bigint.nBytes)) <= 0;
    }

    bool operator==(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return GetLow64() == num;
    }

    bool operator!=(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return GetLow64() != num;
    }

    bool operator>(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return GetLow64() > num;
    }

    bool operator<(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return GetLow64() < num;
    }

    bool operator>=(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return true;
                }
            }
        }
        return GetLow64() >= num;
    }

    bool operator<=(const uint64_t& num) const
    {
        if (nBytes > sizeof(uint64_t)) {
            for (uint32_t i = sizeof(uint64_t); i < nBytes; i++) {
                if (dataPtr[i] != 0) {
                    return false;
                }
            }
        }
        return GetLow64() <= num;
    }

    bool IsInitialized() const
    {
        return dataPtr != nullptr && nBytes != 0;
    }

    void SetNull()
    {
        free(dataPtr);
        dataPtr = nullptr;
        nBytes = 0;
    }

    const uint8_t* GetData() const
    {
        return dataPtr;
    }

    uint64_t GetLow64() const
    {
        uint64_t ret = 0;
        memcpy(&ret, dataPtr, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    uint64_t GetHigh64() const
    {
        const uint32_t bytesSkipped = std::max(int64_t(0), nBytes - (int64_t)sizeof(uint64_t));
        uint64_t ret = 0;
        memcpy(&ret, dataPtr + bytesSkipped, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    uint64_t Get64(const uint32_t& offset) const
    {
        const uint32_t bytesSkipped = std::min(offset, (uint32_t)std::max(int64_t(0), nBytes - (int64_t)sizeof(uint64_t)));
        uint64_t ret = 0;
        memcpy(&ret, dataPtr + bytesSkipped, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    std::string GetHexBE() const
    {
        std::string ret;
        ret.reserve(nBytes * 2);
        for (int64_t i = nBytes - 1; i >= 0; i--) {
            ret.push_back(HEX_CHARS[dataPtr[i] >> 4]);
            ret.push_back(HEX_CHARS[dataPtr[i] & 15]);
        }
        return ret;
    }

    std::string GetHexLE() const
    {
        std::string ret;
        ret.reserve(nBytes * 2);
        for (uint32_t i = 0; i < nBytes; i++) {
            ret.push_back(HEX_CHARS[dataPtr[i] >> 4]);
            ret.push_back(HEX_CHARS[dataPtr[i] & 15]);
        }
        return ret;
    }

    std::string ToString() const
    {
        return GetHexBE();
    }

    uint32_t LengthBytes() const
    {
        return nBytes;
    }

    uint32_t LengthBits() const
    {
        return nBytes * 8;
    }

    const uint8_t* begin() const
    {
        return &dataPtr[0];
    }

    const uint8_t* end() const
    {
        return &dataPtr[nBytes];
    }

    uint8_t* data()
    {
        return dataPtr;
    }
};

#endif // BITCOIN_BIGINTEGER_H
