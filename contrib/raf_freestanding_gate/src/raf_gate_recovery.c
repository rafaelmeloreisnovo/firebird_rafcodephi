/*
 * RAFAELIA receipt-pair verification — optional freestanding experiment.
 * Original contribution offered under the InterBase Public License 1.0.
 * Provided AS IS, without warranty; no Firebird public ABI change.
 *
 * Retain -Wall/-Wextra/-Wconversion/-Werror in scoped CI. Diagnostics are
 * evidence of potential defects, not obstacles to be suppressed.
 */
#include "raf_gate_recovery.h"

/* Constant-time digest comparison for equal-size, public 32-byte buffers.
 * This is integrity checking, not authentication by itself. */
static int digest_equal32(const uint8_t *a, const uint8_t *b)
{
    uint8_t diff = 0u;
    size_t index;
    for (index = 0u; index < RAF_SHA256_BYTES; ++index)
        diff = (uint8_t)(diff | (uint8_t)(a[index] ^ b[index]));
    return diff == 0u;
}

static int terminal_state_is_allowed(uint8_t state)
{
    return state == (uint8_t)RAF_PASS ||
           state == (uint8_t)RAF_ROLLED_BACK ||
           state == (uint8_t)RAF_FAILBACK ||
           state == (uint8_t)RAF_UNRECOVERABLE;
}

raf_pair_status raf_verify_receipt_pair(
    const raf_receipt *prepared, const raf_receipt *terminal,
    uint64_t expected_prepare_sequence,
    const uint8_t expected_parent_sha256[RAF_SHA256_BYTES],
    const uint8_t *evidence, size_t evidence_bytes,
    raf_read_anchor_fn read_anchor, void *anchor_context)
{
    uint64_t observed_sequence = 0u;
    uint8_t observed_digest[RAF_SHA256_BYTES];
    int anchor_status;

    if (!prepared || !terminal || !expected_parent_sha256 ||
        !evidence || evidence_bytes == 0u || evidence_bytes > UINT32_MAX ||
        !read_anchor || expected_prepare_sequence == UINT64_MAX)
        return RAF_PAIR_INVALID_INPUT;

    if (prepared->bytes[13] != (uint8_t)RAF_PREPARED ||
        !raf_receipt_valid(prepared, expected_prepare_sequence,
                           expected_parent_sha256))
        return RAF_PAIR_INVALID_PREPARED;

    if (!terminal_state_is_allowed(terminal->bytes[13]) ||
        !raf_receipt_valid(terminal, expected_prepare_sequence + 1u,
                           &prepared->bytes[RAF_RECEIPT_PREFIX_BYTES]))
        return RAF_PAIR_INVALID_TERMINAL;

    if (!raf_receipt_evidence_match(prepared, evidence, evidence_bytes) ||
        !raf_receipt_evidence_match(terminal, evidence, evidence_bytes))
        return RAF_PAIR_EVIDENCE_MISMATCH;

    anchor_status = read_anchor(anchor_context, &observed_sequence,
                                observed_digest);
    if (anchor_status != 1)
        return RAF_PAIR_ANCHOR_UNAVAILABLE;

    if (observed_sequence != expected_prepare_sequence + 1u ||
        !digest_equal32(observed_digest,
                        &terminal->bytes[RAF_RECEIPT_PREFIX_BYTES]))
        return RAF_PAIR_ANCHOR_MISMATCH;

    return RAF_PAIR_VERIFIED_SCOPED;
}
