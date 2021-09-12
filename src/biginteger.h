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

    // Copy constructor
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

    // Move constructor
    CBigInteger(CBigInteger&& bigint)
    {
        dataPtr = bigint.dataPtr;
        nBytes = bigint.nBytes;

        bigint.dataPtr = nullptr;
        bigint.nBytes = 0;
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

    // Destructor
    ~CBigInteger()
    {
        free(dataPtr);
    }

    // Copy assignment operator
    void operator=(const CBigInteger& bigint)
    {
        if (bigint.IsInitialized()) {
            // free + malloc is faster than realloc for increasing size
            free(dataPtr);

            dataPtr = (uint8_t*)malloc(bigint.nBytes);
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

    // Move assignment operator
    void operator=(CBigInteger&& bigint)
    {
        if (this != &bigint) {
            free(dataPtr);

            dataPtr = bigint.dataPtr;
            nBytes = bigint.nBytes;

            bigint.dataPtr = nullptr;
            bigint.nBytes = 0;
        }
    }

    void operator^=(const CBigInteger& bigint)
    {
        if (nBytes < bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            memset(dataPtr + nBytes, '\0', bigint.nBytes - nBytes);
            nBytes = bigint.nBytes;
        }

        for (uint32_t i = 0; i < bigint.nBytes; i++) {
            dataPtr[i] ^= bigint.dataPtr[i];
        }
    }

    void operator&=(const CBigInteger& bigint)
    {
        if (nBytes > bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            nBytes = bigint.nBytes;
        }

        for (uint32_t i = 0; i < nBytes; i++) {
            dataPtr[i] &= bigint.dataPtr[i];
        }
    }

    void operator|=(const CBigInteger& bigint)
    {
        if (nBytes < bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            memset(dataPtr + nBytes, '\0', bigint.nBytes - nBytes);
            nBytes = bigint.nBytes;
        }

        for (uint32_t i = 0; i < bigint.nBytes; i++) {
            dataPtr[i] |= bigint.dataPtr[i];
        }
    }

    void operator+=(const CBigInteger& bigint)
    {
        if (nBytes < bigint.nBytes) {
            dataPtr = (uint8_t*)realloc(dataPtr, bigint.nBytes);
            memset(dataPtr + nBytes, '\0', bigint.nBytes - nBytes);
            nBytes = bigint.nBytes;
        }

        uint32_t carry = 0;
        for (uint32_t i = 0; i < nBytes; i++) {
            uint32_t n;
            if (i < bigint.nBytes) {
                n = carry + dataPtr[i] + bigint.dataPtr[i];
            } else {
                if (!carry) {
                    return;
                }
                n = carry + dataPtr[i];
            }
            dataPtr[i] = n & 0xff;
            carry = n >> 8;
        }

        // We allocate an extra byte of memory here to hold the result in case overflow would otherwise occur
        if (carry) {
            dataPtr = (uint8_t*)realloc(dataPtr, nBytes + 1);
            dataPtr[nBytes] = carry & 0xff;
            nBytes++;
        }
    }

    void operator-=(const CBigInteger& bigint)
    {
        if (*this <= bigint) {
            memset(dataPtr, '\0', nBytes);
        } else {
            if (nBytes > bigint.nBytes) {
                CBigInteger temp(nBytes, true);
                if (bigint.IsInitialized() && temp.IsInitialized()) {
                    memcpy(temp.dataPtr, bigint.dataPtr, bigint.nBytes);
                    AddWithoutResize(-temp);
                }
            } else {
                AddWithoutResize(-bigint);
            }
            TrimZeroBytes();
        }
    }

    void operator*=(const CBigInteger& bigint)
    {
        // The largest possible number that could be produced by multiplying two numbers with n digits has 2*n digits,
        // so we preallocate a CBigInteger here for this worst case scenario and trim it down later
        uint32_t nBytesNew = nBytes + bigint.nBytes;
        CBigInteger temp(nBytesNew, true);
        if (!temp.IsInitialized()) {
            return;
        }

        for (uint32_t i = 0; i < nBytesNew; i++) {
            uint32_t carry = 0;
            for (uint32_t j = 0; j + i < nBytesNew; j++) {
                uint32_t n;
                if (i < nBytes && j < bigint.nBytes) {
                    n = carry + temp.dataPtr[i + j] + (uint32_t)dataPtr[i] * bigint.dataPtr[j];
                } else {
                    n = carry + temp.dataPtr[i + j];
                }
                temp.dataPtr[i + j] = n & 0xff;
                carry = n >> 8;
            }
        }

        temp.TrimZeroBytes();
        *this = std::move(temp);
    }

    const CBigInteger operator+(const CBigInteger& bigint) const
    {
        CBigInteger ret(*this);
        ret += bigint;
        return ret;
    }

    const CBigInteger operator-(const CBigInteger& bigint) const
    {
        CBigInteger ret(*this);
        ret -= bigint;
        return ret;
    }

    const CBigInteger operator*(const CBigInteger& bigint) const
    {
        CBigInteger ret(*this);
        ret *= bigint;
        return ret;
    }

    void operator=(const uint64_t& num)
    {
        if (nBytes > sizeof(uint64_t)) {
            memset(dataPtr, '\0', nBytes);
        } else if (nBytes < sizeof(uint64_t)) {
            // free + malloc is faster than realloc for increasing size
            free(dataPtr);

            dataPtr = (uint8_t*)malloc(sizeof(uint64_t));
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
        if (nBytes > sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            nBytes = sizeof(uint64_t);
        } else if (nBytes < sizeof(uint64_t)) {
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

    void operator+=(const uint64_t& num)
    {
        if (nBytes < sizeof(uint64_t)) {
            dataPtr = (uint8_t*)realloc(dataPtr, sizeof(uint64_t));
            memset(dataPtr + nBytes, '\0', sizeof(uint64_t) - nBytes);
            nBytes = sizeof(uint64_t);
        }

        uint32_t carry = 0;
        for (uint32_t i = 0; i < nBytes; i++) {
            uint32_t n;
            if (i < sizeof(uint64_t)) {
                n = carry + dataPtr[i] + ((uint8_t*)&num)[i];
            } else {
                if (!carry) {
                    return;
                }
                n = carry + dataPtr[i];
            }
            dataPtr[i] = n & 0xff;
            carry = n >> 8;
        }

        // We allocate an extra byte of memory here to hold the result in case overflow would otherwise occur
        if (carry) {
            dataPtr = (uint8_t*)realloc(dataPtr, nBytes + 1);
            dataPtr[nBytes] = carry & 0xff;
            nBytes++;
        }
    }

    void operator-=(const uint64_t& num)
    {
        if (*this <= num) {
            memset(dataPtr, '\0', nBytes);
        } else {
            if (nBytes > sizeof(uint64_t)) {
                CBigInteger temp(nBytes, true);
                if (temp.IsInitialized()) {
                    memcpy(temp.dataPtr, &num, sizeof(uint64_t));
                    AddWithoutResize(-temp);
                }
            } else {
                AddWithoutResize(~num + 1);
            }
            TrimZeroBytes();
        }
    }

    void operator*=(const uint64_t& num)
    {
        // The largest possible number that could be produced by multiplying two numbers with n digits has 2*n digits,
        // so we preallocate a CBigInteger here for this worst case scenario and trim it down later
        uint32_t nBytesNew = nBytes + sizeof(uint64_t);
        CBigInteger temp(nBytesNew, true);
        if (!temp.IsInitialized()) {
            return;
        }

        for (uint32_t i = 0; i < nBytesNew; i++) {
            uint32_t carry = 0;
            for (uint32_t j = 0; j + i < nBytesNew; j++) {
                uint32_t n;
                if (i < nBytes && j < sizeof(uint64_t)) {
                    n = carry + temp.dataPtr[i + j] + (uint32_t)dataPtr[i] * ((uint8_t*)&num)[j];
                } else {
                    n = carry + temp.dataPtr[i + j];
                }
                temp.dataPtr[i + j] = n & 0xff;
                carry = n >> 8;
            }
        }

        temp.TrimZeroBytes();
        *this = std::move(temp);
    }

    const CBigInteger operator+(const uint64_t& num) const
    {
        CBigInteger ret(*this);
        ret += num;
        return ret;
    }

    const CBigInteger operator-(const uint64_t& num) const
    {
        CBigInteger ret(*this);
        ret -= num;
        return ret;
    }

    const CBigInteger operator*(const uint64_t& num) const
    {
        CBigInteger ret(*this);
        ret *= num;
        return ret;
    }

    const CBigInteger operator~() const
    {
        CBigInteger ret(*this);
        for (uint32_t i = 0; i < ret.nBytes; i++) {
            ret.dataPtr[i] = ~ret.dataPtr[i];
        }
        return ret;
    }

    const CBigInteger operator-() const
    {
        CBigInteger ret(*this);
        for (uint32_t i = 0; i < ret.nBytes; i++) {
            ret.dataPtr[i] = ~ret.dataPtr[i];
        }
        ret.AddWithoutResize(1);
        return ret;
    }

    CBigInteger& operator++()
    {
        // prefix operator
        *this += 1;
        return *this;
    }

    const CBigInteger operator++(int)
    {
        // postfix operator
        const CBigInteger ret(*this);
        *this += 1;
        return ret;
    }

    CBigInteger& operator--()
    {
        // prefix operator
        *this -= 1;
        return *this;
    }

    const CBigInteger operator--(int)
    {
        // postfix operator
        const CBigInteger ret(*this);
        *this -= 1;
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

        // memcmp could be used here if the endianness of dataPtr was easily reversible to compare the largest bytes first
        const uint32_t bytes = std::min(nBytes, bigint.nBytes);

        bool fBytesNotEqual = true;
        for (int64_t i = bytes - 1; i >= 0; i--) {
            if (dataPtr[i] != bigint.dataPtr[i]) {
                if (dataPtr[i] < bigint.dataPtr[i]) {
                    return false;
                }
                break;
            } else if (i == 0) {
                fBytesNotEqual = false;
            }
        }

        return fBytesNotEqual;
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

        // memcmp could be used here if the endianness of dataPtr was easily reversible to compare the largest bytes first
        const uint32_t bytes = std::min(nBytes, bigint.nBytes);

        bool fBytesNotEqual = true;
        for (int64_t i = bytes - 1; i >= 0; i--) {
            if (dataPtr[i] != bigint.dataPtr[i]) {
                if (dataPtr[i] > bigint.dataPtr[i]) {
                    return false;
                }
                break;
            } else if (i == 0) {
                fBytesNotEqual = false;
            }
        }

        return fBytesNotEqual;
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

        // memcmp could be used here if the endianness of dataPtr was easily reversible to compare the largest bytes first
        const uint32_t bytes = std::min(nBytes, bigint.nBytes);

        for (int64_t i = bytes - 1; i >= 0; i--) {
            if (dataPtr[i] != bigint.dataPtr[i]) {
                if (dataPtr[i] < bigint.dataPtr[i]) {
                    return false;
                }
                break;
            }
        }

        return true;
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

        // memcmp could be used here if the endianness of dataPtr was easily reversible to compare the largest bytes first
        const uint32_t bytes = std::min(nBytes, bigint.nBytes);

        for (int64_t i = bytes - 1; i >= 0; i--) {
            if (dataPtr[i] != bigint.dataPtr[i]) {
                if (dataPtr[i] > bigint.dataPtr[i]) {
                    return false;
                }
                break;
            }
        }

        return true;
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

    CBigInteger& BitwiseXor(const CBigInteger& bigint)
    {
        *this ^= bigint;
        return *this;
    }

    CBigInteger& BitwiseAnd(const CBigInteger& bigint)
    {
        *this &= bigint;
        return *this;
    }

    CBigInteger& BitwiseOr(const CBigInteger& bigint)
    {
        *this |= bigint;
        return *this;
    }

    CBigInteger& Add(const CBigInteger& bigint)
    {
        *this += bigint;
        return *this;
    }

    CBigInteger& Subtract(const CBigInteger& bigint)
    {
        *this -= bigint;
        return *this;
    }

    CBigInteger& Multiply(const CBigInteger& bigint)
    {
        *this *= bigint;
        return *this;
    }

    CBigInteger& BitwiseXor(const uint64_t& num)
    {
        *this ^= num;
        return *this;
    }

    CBigInteger& BitwiseAnd(const uint64_t& num)
    {
        *this &= num;
        return *this;
    }

    CBigInteger& BitwiseOr(const uint64_t& num)
    {
        *this |= num;
        return *this;
    }

    CBigInteger& Add(const uint64_t& num)
    {
        *this += num;
        return *this;
    }

    CBigInteger& Subtract(const uint64_t& num)
    {
        *this -= num;
        return *this;
    }

    CBigInteger& Multiply(const uint64_t& num)
    {
        *this *= num;
        return *this;
    }

    CBigInteger& BitwiseInverse()
    {
        for (uint32_t i = 0; i < nBytes; i++) {
            dataPtr[i] = ~dataPtr[i];
        }
        return *this;
    }

    CBigInteger& Negate()
    {
        for (uint32_t i = 0; i < nBytes; i++) {
            dataPtr[i] = ~dataPtr[i];
        }
        AddWithoutResize(1);
        return *this;
    }

    CBigInteger& Increment()
    {
        *this += 1;
        return *this;
    }

    CBigInteger& Decrement()
    {
        *this -= 1;
        return *this;
    }

    CBigInteger& Pow(uint32_t exp)
    {
        if (*this <= 1 || exp == 1) {
            return *this;
        }
        CBigInteger base(*this);
        *this = 1;

        if (!exp) {
            return *this;
        }

        while (true) {
            if (exp & 1) {
                *this *= base;
            }
            exp >>= 1;
            if (!exp) {
                return *this;
            }
            base *= base;
        }
    }

    CBigInteger& AddWithoutResize(const CBigInteger& bigint)
    {
        uint32_t carry = 0;
        for (uint32_t i = 0; i < nBytes; i++) {
            uint32_t n;
            if (i < bigint.nBytes) {
                n = carry + dataPtr[i] + bigint.dataPtr[i];
            } else {
                if (!carry) {
                    return *this;
                }
                n = carry + dataPtr[i];
            }
            dataPtr[i] = n & 0xff;
            carry = n >> 8;
        }

        return *this;
    }

    CBigInteger& AddWithoutResize(const uint64_t& num)
    {
        uint32_t carry = 0;
        for (uint32_t i = 0; i < nBytes; i++) {
            uint32_t n;
            if (i < sizeof(uint64_t)) {
                n = carry + dataPtr[i] + ((uint8_t*)&num)[i];
            } else {
                if (!carry) {
                    return *this;
                }
                n = carry + dataPtr[i];
            }
            dataPtr[i] = n & 0xff;
            carry = n >> 8;
        }

        return *this;
    }

    const CBigInteger Max(const CBigInteger& bigint) const
    {
        if (*this >= bigint) {
            return *this;
        } else {
            return bigint;
        }
    }

    const CBigInteger Min(const CBigInteger& bigint) const
    {
        if (*this <= bigint) {
            return *this;
        } else {
            return bigint;
        }
    }

    const CBigInteger Max(const uint64_t& num) const
    {
        if (*this >= num) {
            return *this;
        } else {
            // CBigInteger constructor for uint64_t called here
            return num;
        }
    }

    const CBigInteger Min(const uint64_t& num) const
    {
        if (*this <= num) {
            return *this;
        } else {
            // CBigInteger constructor for uint64_t called here
            return num;
        }
    }

    inline bool IsInitialized() const
    {
        return dataPtr != nullptr && nBytes != 0;
    }

    inline bool IsNull() const
    {
        return dataPtr == nullptr || nBytes == 0;
    }

    inline void SetNull()
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
        if (!IsInitialized()) {
            return 0;
        }

        uint64_t ret = 0;
        memcpy(&ret, dataPtr, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    uint64_t GetHigh64() const
    {
        if (!IsInitialized()) {
            return 0;
        }

        const uint32_t bytesSkipped = std::max(int64_t(0), nBytes - (int64_t)sizeof(uint64_t));
        uint64_t ret = 0;
        memcpy(&ret, dataPtr + bytesSkipped, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    uint64_t Get64(const uint32_t& offset) const
    {
        if (!IsInitialized()) {
            return 0;
        }

        const uint32_t bytesSkipped = std::min(offset, (uint32_t)std::max(int64_t(0), nBytes - (int64_t)sizeof(uint64_t)));
        uint64_t ret = 0;
        memcpy(&ret, dataPtr + bytesSkipped, std::min(nBytes, (uint32_t)sizeof(uint64_t)));
        return ret;
    }

    void TrimZeroBytes()
    {
        for (int64_t i = nBytes - 1; i >= 0; i--) {
            if (dataPtr[i] || i == 0) {
                const uint32_t nActualBytes = i + 1;
                if (nBytes == nActualBytes) {
                    return;
                }
                dataPtr = (uint8_t*)realloc(dataPtr, nActualBytes);
                nBytes = nActualBytes;
                return;
            }
        }
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

    uint32_t NonzeroBytes() const
    {
        for (int64_t i = nBytes - 1; i >= 0; i--) {
            if (dataPtr[i]) {
                const uint32_t nActualBytes = i + 1;
                return nActualBytes;
            }
        }
        return 0;
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
