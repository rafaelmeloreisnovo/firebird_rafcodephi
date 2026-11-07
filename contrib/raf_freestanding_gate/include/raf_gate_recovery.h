/*
 * RAFAELIA receipt-pair verification — optional freestanding experiment.
 * Original contribution offered under the InterBase Public License 1.0.
 * Provided AS IS, without warranty; no Firebird public ABI change.
 *
 * Keep warnings visible: unexpected conversions or bounds are falsifiers.
 * No libc, heap, I/O, clock, filesystem or OS dependency in this component.
 */
#ifndef RAF_GATE_RECOVERY_H
#define RAF_GATE_RECOVERY_H
#include "raf_gate.h"

/* Read the independently trusted CURRENT receipt anchor.
 * Returns 1 only on authenticated, fresh, authoritative readback.
 * All non-1 results are unavailable/indeterminate and block verification.
 * Returning a cached value or the caller's supplied receipt is NOT sufficient.
 */
typedef int (*raf_read_anchor_fn)(void *context, uint64_t *current_sequence,
                                  uint8_t current_digest[RAF_SHA256_BYTES]);

typedef enum raf_pair_status {
    RAF_PAIR_INVALID_INPUT = 0,
    RAF_PAIR_INVALID_PREPARED = 1,
    RAF_PAIR_INVALID_TERMINAL = 2,
    RAF_PAIR_EVIDENCE_MISMATCH = 3,
    RAF_PAIR_ANCHOR_UNAVAILABLE = 4,
    RAF_PAIR_ANCHOR_MISMATCH = 5,
    RAF_PAIR_VERIFIED_SCOPED = 6
} raf_pair_status;

/* Read-only receipt verifier, NOT a recovery executor or authorization gate.
 * A successful result confirms record integrity, linkage and equality with
 * a trusted current anchor observed during this call, NOT sink acceptance,
 * concurrency fencing, durable atomicity or hardware-backed rollback safety.
 */
raf_pair_status raf_verify_receipt_pair(
    const raf_receipt *prepared, const raf_receipt *terminal,
    uint64_t expected_prepare_sequence,
    const uint8_t expected_parent_sha256[RAF_SHA256_BYTES],
    const uint8_t *evidence, size_t evidence_bytes,
    raf_read_anchor_fn read_anchor, void *anchor_context);
#endif
