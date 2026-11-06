/* SPDX-License-Identifier: MIT
 * Pure deterministic observation. No calls to OS, libc or Firebird engine.
 */
#include "../include/raf_guard.h"

static raf_result unknown_result(uint64_t sequence, raf_reason why, uint64_t interval)
{
    raf_result r;
    r.sequence = sequence;
    r.recommended_interval = interval;
    r.state = RAF_UNKNOWN;
    r.action = RAF_NONE;
    r.reason = why;
    return r;
}

int raf_policy_valid(const raf_policy* p)
{
    return p && p->suspect_ticks && p->stalled_ticks > p->suspect_ticks &&
        p->min_interval && p->max_interval >= p->min_interval;
}

void raf_guard_reset(raf_guard* g)
{
    if (!g) return;
    g->sequence = 0;
    g->last_tick = 0;
    g->last_progress = 0;
    g->last_progress_tick = 0;
    g->initialized = 0;
}

raf_result raf_guard_step(raf_guard* g, const raf_policy* p, const raf_sample* s)
{
    raf_result r = unknown_result(g ? g->sequence : 0, RAF_REASON_NO_DATA,
                                  raf_policy_valid(p) ? p->min_interval : 0);
    if (!g || !p || !s) return r;
    if (!raf_policy_valid(p)) {
        g->initialized = 0;
        r.reason = RAF_REASON_BAD_POLICY;
        return r;
    }
    if (g->sequence == UINT64_MAX) {
        g->initialized = 0;
        r.reason = RAF_REASON_SEQ_EXHAUSTED;
        return r;
    }
    ++g->sequence;
    r.sequence = g->sequence;
    if ((s->known & (RAF_KNOWN_TICK | RAF_KNOWN_PROGRESS)) !=
            (RAF_KNOWN_TICK | RAF_KNOWN_PROGRESS) ||
        (s->known & ~((uint32_t) RAF_KNOWN_ALL)) != 0 ||
        ((s->known & RAF_KNOWN_IO) && s->io_failed > 1u)) {
        g->initialized = 0;
        return r;
    }

    if (g->initialized && s->tick < g->last_tick) {
        g->initialized = 0;
        r.reason = RAF_REASON_CLOCK_REWIND;
        return r;
    }
    if (g->initialized && s->progress < g->last_progress) {
        g->initialized = 0;
        r.reason = RAF_REASON_COUNTER_REWIND;
        return r;
    }

    if (!g->initialized) {
        g->last_progress_tick = s->tick;
        g->initialized = 1;
        r.reason = RAF_REASON_INITIAL;
    } else if (s->progress != g->last_progress) {
        g->last_progress_tick = s->tick;
        r.reason = RAF_REASON_PROGRESS;
    } else {
        r.reason = RAF_REASON_NO_PROGRESS;
    }

    g->last_tick = s->tick;
    g->last_progress = s->progress;

    if ((s->known & RAF_KNOWN_IO) && s->io_failed) {
        r.state = RAF_IO_ERROR;
        r.action = RAF_ALERT;
        r.reason = RAF_REASON_IO_ERROR;
    } else {
        const uint64_t no_progress = s->tick - g->last_progress_tick;
        if (no_progress >= p->stalled_ticks) {
            r.state = RAF_STALLED;
            r.action = RAF_ALERT;
            r.reason = RAF_REASON_STALL;
        } else if (no_progress >= p->suspect_ticks) {
            r.state = RAF_SUSPECT;
            r.action = RAF_OBSERVE;
        } else {
            r.state = RAF_HEALTHY;
            r.action = RAF_OBSERVE;
        }
    }
    r.recommended_interval = (r.state == RAF_HEALTHY) ? p->max_interval : p->min_interval;
    return r;
}

static void put64(uint8_t* b, uint64_t n)
{
    unsigned i;
    for (i = 0; i < 8u; ++i) {
        b[i] = (uint8_t) (n & UINT64_C(255));
        n >>= 8;
    }
}

static uint32_t get32(const uint8_t* b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
        ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static uint32_t crc32_bytes(const uint8_t* b, unsigned n)
{
    uint32_t crc = UINT32_MAX;
    unsigned i, j;
    for (i = 0u; i < n; ++i) {
        crc ^= b[i];
        for (j = 0u; j < 8u; ++j)
            crc = (crc >> 1) ^ ((crc & 1u) ? UINT32_C(0xedb88320) : 0u);
    }
    return crc ^ UINT32_MAX;
}

int raf_frame_encode(const raf_result* r, const raf_sample* s, uint8_t out[RAF_FRAME_SIZE])
{
    unsigned i;
    uint32_t crc;
    if (!r || !s || !out || r->state > RAF_IO_ERROR || r->action > RAF_ALERT ||
        r->reason > RAF_REASON_SEQ_EXHAUSTED || (s->known & ~((uint32_t) RAF_KNOWN_ALL))) return 0;
    for (i = 0u; i < RAF_FRAME_SIZE; ++i) out[i] = 0;
    out[0] = 'R'; out[1] = 'A'; out[2] = 'F'; out[3] = 'G';
    out[4] = RAF_GUARD_VERSION;
    out[5] = (uint8_t) s->known;
    out[6] = (uint8_t) r->state;
    out[7] = (uint8_t) r->action;
    put64(out + 8u, r->sequence);
    put64(out + 16u, (s->known & RAF_KNOWN_TICK) ? s->tick : 0u);
    put64(out + 24u, (s->known & RAF_KNOWN_PROGRESS) ? s->progress : 0u);
    put64(out + 32u, r->recommended_interval);
    out[40] = (uint8_t) r->reason;
    crc = crc32_bytes(out, 44u);
    out[44] = (uint8_t) crc;
    out[45] = (uint8_t) (crc >> 8);
    out[46] = (uint8_t) (crc >> 16);
    out[47] = (uint8_t) (crc >> 24);
    return 1;
}

int raf_frame_verify(const uint8_t in[RAF_FRAME_SIZE])
{
    if (!in) return 0;
    if (in[0] != 'R' || in[1] != 'A' || in[2] != 'F' || in[3] != 'G' ||
        in[4] != RAF_GUARD_VERSION || (in[5] & ~RAF_KNOWN_ALL) ||
        in[6] > RAF_IO_ERROR || in[7] > RAF_ALERT ||
        in[40] > RAF_REASON_SEQ_EXHAUSTED ||
        in[41] || in[42] || in[43]) return 0;
    return get32(in + 44u) == crc32_bytes(in, 44u);
}
