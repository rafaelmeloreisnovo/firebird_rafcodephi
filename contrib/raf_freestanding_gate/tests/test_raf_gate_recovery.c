/*
 * Hosted tests ONLY. Optional freestanding verifier has no libc imports.
 * Assertion tests model a trusted external anchor; they DO NOT provide one.
 */
#include "raf_gate_recovery.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct anchor_fixture {
    uint64_t sequence;
    uint8_t digest[RAF_SHA256_BYTES];
    int status;
    unsigned calls;
} anchor_fixture;

static int read_anchor(void *context, uint64_t *sequence,
                       uint8_t digest[RAF_SHA256_BYTES])
{
    anchor_fixture *fixture = (anchor_fixture *)context;
    size_t index;
    fixture->calls++;
    if (fixture->status != 1)
        return fixture->status;
    *sequence = fixture->sequence;
    for (index = 0u; index < RAF_SHA256_BYTES; ++index)
        digest[index] = fixture->digest[index];
    return 1;
}

static void construct(raf_receipt *prepared, raf_receipt *terminal,
                      anchor_fixture *anchor, raf_state terminal_state)
{
    static const uint8_t evidence[] = {'B','L','O','B','-','9','2','5','1'};
    static const uint8_t initial_digest[RAF_SHA256_BYTES] = {0u};
    assert(raf_receipt_create(prepared, 42u, RAF_PREPARED,
                              initial_digest, evidence, sizeof(evidence)));
    assert(raf_receipt_create(terminal, 43u, terminal_state,
                              &prepared->bytes[RAF_RECEIPT_PREFIX_BYTES],
                              evidence, sizeof(evidence)));
    anchor->sequence = 43u;
    memcpy(anchor->digest, &terminal->bytes[RAF_RECEIPT_PREFIX_BYTES],
           RAF_SHA256_BYTES);
    anchor->status = 1;
    anchor->calls = 0u;
}

static raf_pair_status verify(const raf_receipt *prepared,
                              const raf_receipt *terminal,
                              anchor_fixture *anchor)
{
    static const uint8_t evidence[] = {'B','L','O','B','-','9','2','5','1'};
    static const uint8_t initial_digest[RAF_SHA256_BYTES] = {0u};
    return raf_verify_receipt_pair(prepared, terminal, 42u, initial_digest,
                                    evidence, sizeof(evidence),
                                    read_anchor, anchor);
}

int main(void)
{
    static const uint8_t evidence[] = {'B','L','O','B','-','9','2','5','1'};
    static const uint8_t altered_evidence[] = {'B','L','O','B','-','9','2','5','2'};
    static const uint8_t initial_digest[RAF_SHA256_BYTES] = {0u};
    static const uint8_t wrong_digest[RAF_SHA256_BYTES] = {1u};
    raf_receipt prepared, terminal, variant;
    anchor_fixture anchor;
    unsigned tests = 0u;

    construct(&prepared, &terminal, &anchor, RAF_PASS);
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_VERIFIED_SCOPED);
    assert(anchor.calls == 1u); ++tests;

    construct(&prepared, &terminal, &anchor, RAF_ROLLED_BACK);
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_VERIFIED_SCOPED); ++tests;

    construct(&prepared, &terminal, &anchor, RAF_FAILBACK);
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_VERIFIED_SCOPED); ++tests;

    construct(&prepared, &terminal, &anchor, RAF_UNRECOVERABLE);
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_VERIFIED_SCOPED); ++tests;

    construct(&prepared, &terminal, &anchor, RAF_PASS);
    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u, wrong_digest,
        evidence, sizeof(evidence), read_anchor, &anchor) == RAF_PAIR_INVALID_PREPARED);
    assert(anchor.calls == 0u); ++tests;

    variant = prepared;
    variant.bytes[13] = (uint8_t)RAF_PASS;
    assert(verify(&variant, &terminal, &anchor) == RAF_PAIR_INVALID_PREPARED); ++tests;

    variant = terminal;
    variant.bytes[13] = (uint8_t)RAF_PREPARED;
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    variant = terminal;
    variant.bytes[13] = (uint8_t)RAF_BLOCKED;
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    variant = terminal;
    variant.bytes[12] ^= 1u;
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    variant = terminal;
    variant.bytes[53] ^= 1u;
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    variant = prepared;
    variant.bytes[51] ^= 1u;
    assert(verify(&variant, &terminal, &anchor) == RAF_PAIR_INVALID_PREPARED); ++tests;

    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u,
        initial_digest, altered_evidence, sizeof(altered_evidence),
        read_anchor, &anchor) == RAF_PAIR_EVIDENCE_MISMATCH); ++tests;

    anchor.digest[0] ^= 1u;
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_ANCHOR_MISMATCH); ++tests;
    anchor.digest[0] ^= 1u;

    anchor.sequence = 44u;
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_ANCHOR_MISMATCH); ++tests;
    anchor.sequence = 43u;

    anchor.status = 0;
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_ANCHOR_UNAVAILABLE); ++tests;
    anchor.status = -1;
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_ANCHOR_UNAVAILABLE); ++tests;
    anchor.status = 1;

    assert(raf_verify_receipt_pair(&prepared, &terminal, UINT64_MAX,
        initial_digest, evidence, sizeof(evidence), read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u,
        initial_digest, NULL, sizeof(evidence), read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u,
        initial_digest, evidence, 0u, read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u,
        initial_digest, evidence, sizeof(evidence), NULL,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(NULL, &terminal, 42u,
        initial_digest, evidence, sizeof(evidence), read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(&prepared, NULL, 42u,
        initial_digest, evidence, sizeof(evidence), read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    assert(raf_verify_receipt_pair(&prepared, &terminal, 42u,
        NULL, evidence, sizeof(evidence), read_anchor,
        &anchor) == RAF_PAIR_INVALID_INPUT); ++tests;

    variant = terminal;
    variant.bytes[14] ^= 1u; /* break the explicit PREPARED -> terminal link */
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    assert(raf_receipt_create(&variant, 43u, RAF_PASS, initial_digest,
                              evidence, sizeof(evidence)));
    assert(verify(&prepared, &variant, &anchor) == RAF_PAIR_INVALID_TERMINAL); ++tests;

    /* Even a valid receipt with an outdated authoritative anchor must fail. */
    anchor.sequence = 42u;
    assert(verify(&prepared, &terminal, &anchor) == RAF_PAIR_ANCHOR_MISMATCH); ++tests;

    printf("PASS_SCOPED_HOST: %u receipt-pair/anchor tests\n", tests);
    return 0;
}
