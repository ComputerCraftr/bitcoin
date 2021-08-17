// Copyright (c) 2018-2021 John "ComputerCraftr" Studnicka
// Copyright (c) 2018-2020 The Simplicity developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_SCRYPT_H
#define BITCOIN_CRYPTO_SCRYPT_H

#include <stdlib.h>
#include <stdint.h>

static const uint32_t SCRYPT_SCRATCHPAD_SIZE = 1024 * 128 + 63; // N = 1024
static const uint32_t SCRYPT2_SCRATCHPAD_SIZE = 1048576 * 128 + 63; // N = 1024^2 = 1048576

void scrypt_N_1_1_256(const char *input, unsigned int inputLen, char *output, unsigned int N);
void scrypt_N_1_1_256_sp_generic(const char *input, unsigned int inputLen, char *output, char *scratchpad, unsigned int N);

#if defined(USE_SSE2)
#include <string>
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
#define USE_SSE2_ALWAYS 1
#define scrypt_N_1_1_256_sp(input, inputLen, output, scratchpad, N) scrypt_N_1_1_256_sp_sse2((input), (inputLen), (output), (scratchpad), (N))
#else
#define scrypt_N_1_1_256_sp(input, inputLen, output, scratchpad, N) scrypt_N_1_1_256_sp_detected((input), (inputLen), (output), (scratchpad), (N))
#endif

std::string scrypt_detect_sse2();
void scrypt_N_1_1_256_sp_sse2(const char *input, unsigned int inputLen, char *output, char *scratchpad, unsigned int N);
extern void (*scrypt_N_1_1_256_sp_detected)(const char *input, unsigned int inputLen, char *output, char *scratchpad, unsigned int N);
#else
#define scrypt_N_1_1_256_sp(input, inputLen, output, scratchpad, N) scrypt_N_1_1_256_sp_generic((input), (inputLen), (output), (scratchpad), (N))
#endif

#ifndef __FreeBSD__
static inline uint32_t le32dec(const void *pp)
{
    const uint8_t *p = (uint8_t const *)pp;
    return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
        ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void le32enc(void *pp, uint32_t x)
{
    uint8_t *p = (uint8_t *)pp;
    p[0] = x & 0xff;
    p[1] = (x >> 8) & 0xff;
    p[2] = (x >> 16) & 0xff;
    p[3] = (x >> 24) & 0xff;
}
#endif
#endif // BITCOIN_CRYPTO_SCRYPT_H
