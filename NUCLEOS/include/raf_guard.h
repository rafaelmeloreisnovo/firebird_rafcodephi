/* SPDX-License-Identifier: MIT
 * Standalone RAFAELIA observer kernel. No Firebird code included.
 */
#ifndef RAF_GUARD_H
#define RAF_GUARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAF_GUARD_VERSION 1u
#define RAF_FRAME_SIZE 48u
#define RAF_KNOWN_TICK 1u
#define RAF_KNOWN_PROGRESS 2u
#define RAF_KNOWN_IO 4u
#define RAF_KNOWN_ALL 7u

typedef enum raf_state {
    RAF_UNKNOWN = 0,
    RAF_HEALTHY = 1,
    RAF_SUSPECT = 2,
    RAF_STALLED = 3,
    RAF_IO_ERROR = 4
} raf_state;

typedef enum raf_action {
    RAF_NONE = 0,
    RAF_OBSERVE = 1,
    RAF_ALERT = 2
} raf_action;

typedef enum raf_reason {
    RAF_REASON_NO_DATA = 0,
    RAF_REASON_INITIAL = 1,
    RAF_REASON_PROGRESS = 2,
    RAF_REASON_NO_PROGRESS = 3,
    RAF_REASON_STALL = 4,
    RAF_REASON_IO_ERROR = 5,
    RAF_REASON_CLOCK_REWIND = 6,
    RAF_REASON_COUNTER_REWIND = 7,
    RAF_REASON_BAD_POLICY = 8,
    RAF_REASON_SEQ_EXHAUSTED = 9
} raf_reason;

typedef struct raf_policy {
    uint64_t suspect_ticks;  /* > 0 */
    uint64_t stalled_ticks;  /* > suspect_ticks */
    uint64_t min_interval;   /* > 0 */
    uint64_t max_interval;   /* >= min_interval */
} raf_policy;

typedef struct raf_sample {
    uint64_t tick;           /* Trusted monotonic clock, units set by adapter */
    uint64_t progress;       /* Trusted monotonic completed-operation counter */
    uint32_t known;          /* Unknown is NOT zero */
    uint8_t io_failed;       /* Consult only when RAF_KNOWN_IO is set */
} raf_sample;

typedef struct raf_guard {
    uint64_t sequence;
    uint64_t last_tick;
    uint64_t last_progress;
    uint64_t last_progress_tick;
    uint8_t initialized;
} raf_guard;

typedef struct raf_result {
    uint64_t sequence;
    uint64_t recommended_interval;
    raf_state state;
    raf_action action;
    raf_reason reason;
} raf_result;

int raf_policy_valid(const raf_policy* policy);
void raf_guard_reset(raf_guard* guard);
raf_result raf_guard_step(raf_guard* guard, const raf_policy* policy, const raf_sample* sample);

/* Fixed-format public diagnostic frame. CRC32 is NOT cryptographic or authenticating. */
int raf_frame_encode(const raf_result* result, const raf_sample* sample, uint8_t out[RAF_FRAME_SIZE]);
int raf_frame_verify(const uint8_t in[RAF_FRAME_SIZE]);

#ifdef __cplusplus
}
#endif
#endif
