// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow_factorization.h>

#include <crypto/common.h>
#include <crypto/pbkdf2_hmac.h>
#include <primes_list.h>

bool CheckPrimeFactorization(const uint256& hashPrevBlock, const uint32_t& nBits, const std::vector<unsigned char>& vPrimeFactors)
{
    // TODO: support factoring unlimited length integers
    if (nBits > 512) {
        return false;
    }
    const uint32_t nBytes = (nBits + 7) / 8; // round up while converting bits to bytes
    const uint32_t nBitsRemainder = nBits % 8;
    arith_uint512 integerToFactor;
    arith_uint512 integerToCheck(1);
    uint32_t nBitsLE;
    WriteLE32((unsigned char*)&nBitsLE, nBits); // bits is hashed in little endian format
    pbkdf2_hmac_sha256((const unsigned char*)&hashPrevBlock, 32, (const unsigned char*)&nBitsLE, 4, 1, nBytes, (unsigned char*)&integerToFactor);
    if (nBitsRemainder) {
        integerToFactor >>= 8 - nBitsRemainder; // adjust the length to the proper number of bits
    }
    integerToFactor |= integerToCheck << (nBits - 1); // ensure the top bit of integerToFactor is always set so that the hmac sha256 hash cannot be "mined" to produce a smaller number for factoring
    integerToFactor |= 1u; // ensure integerToFactor is always an odd number by setting the bottom bit

    arith_uint512 lastFactor;
    uint64_t currentPos = 0;
    while (currentPos < vPrimeFactors.size()) {
        if (currentPos + vPrimeFactors[currentPos] >= vPrimeFactors.size()) {
            return false;
        }
        // TODO: support unlimited length factors
        if (vPrimeFactors[currentPos] > 64 || vPrimeFactors[currentPos] == 0) {
            return false;
        }

        arith_uint512 factor;
        memcpy((void*)&factor, vPrimeFactors.data()+currentPos+1, vPrimeFactors[currentPos]); // read factor from byte vector
        if (factor <= lastFactor) {
            return false;
        }
        if (!IsPrime(factor)) {
            return false;
        }

        currentPos += vPrimeFactors[currentPos] + 1; // skip ahead to factor count length byte
        if (currentPos + vPrimeFactors[currentPos] >= vPrimeFactors.size()) {
            return false;
        }
        if (vPrimeFactors[currentPos] > 4 || vPrimeFactors[currentPos] == 0) {
            return false;
        }

        uint32_t numOfFactor = 0;
        memcpy(&numOfFactor, vPrimeFactors.data()+currentPos+1, vPrimeFactors[currentPos]); // read factor count from byte vector
        numOfFactor = le32toh(numOfFactor); // number is stored in little endian format
        for (uint32_t i = 0; i < numOfFactor; i++) {
            integerToCheck *= factor;
        }

        currentPos += vPrimeFactors[currentPos] + 1; // skip ahead to next factor length byte
        lastFactor = factor;
    }

    return integerToFactor == integerToCheck;
}

bool AKSPrimalityTest(const arith_uint512& nFactor)
{
    // TODO
    return false;
}

// TODO: replace arith_uint512 to allow this function to handle unlimited length integers
bool IsPrime(const arith_uint512& nFactor)
{
    static const arith_uint512 UINT8_MAX_VALUE(UINT8_MAX);
    static const arith_uint512 UINT16_MAX_VALUE(UINT16_MAX);
    const uint64_t& nFactorLowBits = nFactor.GetLow64();

    if (!(nFactorLowBits & 1u)) {
        return nFactor == 2; // all primes are odd with the exception of 2
    }

    if (nFactor <= UINT8_MAX_VALUE) {
        for (uint32_t i = 0; i < COUNT_PRIMES_8_BIT; i++) {
            if (nFactorLowBits == PRIMES_8_BIT[i]) {
                return true;
            }
        }
    } else if (nFactor <= UINT16_MAX_VALUE) {
        for (uint32_t i = 0; i < COUNT_PRIMES_16_BIT; i++) {
            if (nFactorLowBits == PRIMES_16_BIT[i]) {
                return true;
            }
        }
    } else {
        return AKSPrimalityTest(nFactor);
    }

    return false;
}
