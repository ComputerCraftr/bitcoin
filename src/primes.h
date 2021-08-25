// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMES_H
#define BITCOIN_PRIMES_H

#include <uint256.h>

#include <stdint.h>
#include <vector>

bool CheckPrimeFactorization(const uint256& hashPrevBlock, const unsigned int& nBits, const std::vector<unsigned char>& vPrimeFactors);

#endif // BITCOIN_PRIMES_H
