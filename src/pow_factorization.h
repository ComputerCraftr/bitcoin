// Copyright (c) 2021 John "ComputerCraftr" Studnicka
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_FACTORIZATION_H
#define BITCOIN_POW_FACTORIZATION_H

#include <arith_uint256.h>
#include <uint256.h>

#include <stdint.h>
#include <vector>

bool IsPrime(const arith_uint512& factor);

bool CheckPrimeFactorization(const uint256& hashPrevBlock, const unsigned int& nBits, const std::vector<unsigned char>& vPrimeFactors);

#endif // BITCOIN_POW_FACTORIZATION_H
