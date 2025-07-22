// Copyright (c) 2020-2025 The XEP Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELECTRAPROTOCOL_CONSENSUS_TX_CHECK_H
#define ELECTRAPROTOCOL_CONSENSUS_TX_CHECK_H

#include <consensus/validation.h>

bool XEP_CheckTransactionSize(TxValidationState& state, size_t size)
{
    // 64-byte transactions are rejected to mitigate CVE-2017-12842
    if (size == 64) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "tx-size-small");
    } else {
        return true;
    }
};

#endif // ELECTRAPROTOCOL_CONSENSUS_TX_CHECK_H
