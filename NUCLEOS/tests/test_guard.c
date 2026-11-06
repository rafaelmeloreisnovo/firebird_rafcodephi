/* SPDX-License-Identifier: MIT
 * Hosted synthetic tests; hosted stdio is NOT part of the freestanding kernel.
 */
#include "../include/raf_guard.h"
#include <stdio.h>

static unsigned tests = 0;
#define CHECK(x) do { ++tests; if (!(x)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; \
} } while (0)

static raf_sample sample(uint64_t t, uint64_t prog, uint32_t known, uint8_t io)
{
    raf_sample s = { t, prog, known, io };
    return s;
}

int main(void)
{
    raf_policy p = { 10, 30, 1, 8 };
    raf_guard g;
    raf_result r;
    uint8_t frame[RAF_FRAME_SIZE];
    unsigned i;
    CHECK(raf_policy_valid(&p));
    CHECK(!raf_policy_valid(0));
    raf_guard_reset(&g);
    CHECK(g.sequence == 0 && g.initialized == 0);
    {
        raf_sample s = sample(100, 0, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_HEALTHY && r.reason == RAF_REASON_INITIAL);
        CHECK(r.action == RAF_OBSERVE && r.recommended_interval == 8);
        CHECK(r.sequence == 1);
        CHECK(raf_frame_encode(&r, &s, frame));
        CHECK(raf_frame_verify(frame));
        CHECK(frame[0] == 'R' && frame[3] == 'G' && frame[4] == 1);
        CHECK(frame[24] == 0); /* A valid zero progress is encoded */
        frame[27] ^= 1u; /* accidental corruption */
        CHECK(!raf_frame_verify(frame));
        frame[27] ^= 1u;
        CHECK(raf_frame_verify(frame));
        frame[41] ^= 1u; CHECK(!raf_frame_verify(frame)); frame[41] ^= 1u;
        CHECK(raf_frame_verify(frame));
    }
    {
        raf_sample s = sample(110, 0, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_SUSPECT && r.action == RAF_OBSERVE);
        CHECK(r.recommended_interval == 1);
    }
    {
        raf_sample s = sample(130, 0, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_STALLED && r.action == RAF_ALERT);
        CHECK(r.reason == RAF_REASON_STALL);
    }
    {
        raf_sample s = sample(131, 1, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_HEALTHY && r.reason == RAF_REASON_PROGRESS);
    }
    {
        raf_sample s = sample(132, 2, RAF_KNOWN_ALL, 1);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_IO_ERROR && r.action == RAF_ALERT);
        CHECK(r.reason == RAF_REASON_IO_ERROR);
    }
    {
        raf_sample s = sample(133, 3, RAF_KNOWN_TICK | RAF_KNOWN_PROGRESS, 1);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_HEALTHY); /* unknown IO != positive IO error */
    }
    {
        raf_sample s = sample(134, 4, RAF_KNOWN_TICK, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN && r.action == RAF_NONE);
        CHECK(!g.initialized);
        CHECK(raf_frame_encode(&r, &s, frame));
        CHECK(frame[24] == 0); /* missing progress is redacted to zero */
    }
    {
        raf_sample s = sample(135, 5, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.reason == RAF_REASON_INITIAL); /* fresh baseline */
        s.tick = 134;
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN && r.reason == RAF_REASON_CLOCK_REWIND);
        CHECK(!g.initialized);
    }
    {
        raf_sample s = sample(145, 10, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s); CHECK(r.state == RAF_HEALTHY);
        s.tick = 146; s.progress = 9;
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN && r.reason == RAF_REASON_COUNTER_REWIND);
    }
    {
        raf_sample s = sample(150, 20, RAF_KNOWN_ALL | 8u, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN && r.action == RAF_NONE);
        CHECK(!raf_frame_encode(&r, &s, frame));
        s.known = RAF_KNOWN_ALL; s.io_failed = 2;
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN);
    }
    {
        raf_policy bad = p;
        raf_sample s = sample(151, 20, RAF_KNOWN_ALL, 0);
        bad.stalled_ticks = bad.suspect_ticks;
        r = raf_guard_step(&g, &bad, &s);
        CHECK(r.state == RAF_UNKNOWN && r.reason == RAF_REASON_BAD_POLICY);
        CHECK(!raf_policy_valid(&bad));
        bad = p; bad.max_interval = 0;
        CHECK(!raf_policy_valid(&bad));
    }
    {
        raf_sample s = sample(160, 22, RAF_KNOWN_ALL, 0);
        g.sequence = UINT64_MAX;
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.state == RAF_UNKNOWN && r.reason == RAF_REASON_SEQ_EXHAUSTED);
        CHECK(g.sequence == UINT64_MAX && !g.initialized);
    }
    CHECK(raf_frame_verify(frame) == 1); /* failed encode preserves previous valid frame */
    frame[5] |= 8u; /* reject unrecognized known-field bits */
    CHECK(!raf_frame_verify(frame));
    frame[5] &= (uint8_t)~8u;
    CHECK(raf_frame_verify(frame));
    CHECK(!raf_frame_verify(0));
    CHECK(!raf_frame_encode(0, 0, 0));
    /* Adversarial regression: sustained empty progress must never emit a recovery action. */
    raf_guard_reset(&g);
    for (i = 0u; i < 10000u; ++i) {
        raf_sample s = sample((uint64_t)i * 10u, 0, RAF_KNOWN_ALL, 0);
        r = raf_guard_step(&g, &p, &s);
        CHECK(r.action == RAF_NONE || r.action == RAF_OBSERVE || r.action == RAF_ALERT);
        CHECK(r.recommended_interval >= p.min_interval && r.recommended_interval <= p.max_interval);
        if (i >= 3u) CHECK(r.state == RAF_STALLED && r.action == RAF_ALERT);
    }
    printf("NUCLEOS_GUARD_PASS assertions=%u; synthetic_samples=10000; Firebird_E2E=NOT_RUN\n", tests);
    return 0;
}
