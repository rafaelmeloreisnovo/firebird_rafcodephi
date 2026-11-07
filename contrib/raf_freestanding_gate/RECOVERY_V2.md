# Evidence gate V2 — authenticated-current receipt verification (scope: research)

## Why V1 is not enough

The V1 receipt chain verifies local SHA-256 integrity and predecessor linkage,
but by itself it cannot distinguish an authentic **stale** copy of a receipt
from the current authoritative receipt. It also cannot know whether an
independent destination accepted a side effect just before a crash.

This extension does NOT claim to solve distributed exactly-once delivery.
It introduces a small, optional, read-only **pair verification** primitive,
independent of Firebird protocol code and its production release gates.

- `raf_gate_recovery.h`: pinned pair-verification contract.
- `raf_gate_recovery.c`: all-C11 freestanding pure checks + a caller-supplied,
  authenticated-current anchor callback; zero libc, OS, heap or network imports.
- `test_raf_gate_recovery.c`: adversarial hosted tests (tamper, stale anchor,
  mismatched sequence/parent, missing authority, missing evidence).
- Separate scoped GitHub CI runs GCC and Clang and inspects undefined symbols.

## Fail-closed invariant

```text
PREPARED(seq=n, previous=P, evidence=H(E))
   |
   +--> TERMINAL(seq=n+1, previous=SHA256(PREPARED), evidence=H(E))
                  |
                  +--> trusted_authority_current_anchor
                       {sequence=n+1, sha256=SHA256(TERMINAL)}
                               |
                               v
                       VERIFIED_SCOPED
```

A caller MUST supply a genuinely independent and *current* authenticated
readback callback `raf_read_anchor_fn`, not an echo of local data. The core
does not implement such a source. The callback MUST reject stale snapshots,
stale authority epochs, read errors and inconclusive results, and return 1
only when its own external checks are satisfied.

The verifier returns typed failure states, checking all fields before
querying the external authority. It verifies *only a snapshot* at the time
the callback executes. If a subsequent side effect is attempted, it
requires an external CAS/fencing/authorization gate; there is no
time-of-check/time-of-use guarantee in this module.

An independently accepted sink operation requires *sink-side* acceptance
evidence (or an explicit idempotent commit protocol). Source-only logging
cannot decide between "accepted-but-not-checkpointed" and
"not-accepted" after a crash. This is not an optional optimization.

An auth-current readback is not implemented by the test harness: tests
supply an explicitly marked stub. Production integration remains blocked.

## Literature and falsifiers

- Andreakis, *Machine-Checked Dual-Write Recovery from a Commit Log*,
  [arXiv:2608.00501](https://arxiv.org/abs/2608.00501)
  (v5 dated 2026-09-11 in source), studies the ambiguity of source-only
  crash recovery and requires complete/current sink acceptance evidence,
  arrival/claim fencing and retained identities for conditional guarantees.
  Its model does not certify this implementation.
- Burke et al., *It's a Feature, Not a Bug: Secure and Auditable State
  Rollback for Confidential Cloud Applications*,
  [arXiv:2511.13641](https://arxiv.org/abs/2511.13641)
  describes authorized rollback governed by a reference monitor, with
  atomic transitions and tamper-evident audit logs; it assumes additional
  trusted hardware/policy conditions absent here.

**Falsifiers for production promotion:** an accepted stale anchor; success
despite mismatched evidence/previous digest; success when the verifier has
no independent authority; missing sink acceptance record after dual-write
crash; unbounded trust in cached anchors; ABI or build regression; inability
to identify the actor responsible for rollback.

## Status matrix

| Layer | Status |
|---|---|
| C11 freestanding read-only structural receipt verification | Implemented in isolated research branch; must read CI results |
| Hosted GCC/Clang adversarial tests | Implemented in CI workflow; terminal result requires provider readback |
| Authenticated-current external anchor provider | TOKEN_VAZIO / NOT_IMPLEMENTED |
| Sink acceptance/fencing and identity retention | TOKEN_VAZIO / NOT_IMPLEMENTED |
| Firebird 4/5/master packet-boundary BLOB E2E | TOKEN_VAZIO / NOT_RUN |
| ISO, IEEE, RFC or W3C full compliance certification | NOT_CLAIMED |
| Master upstream issue-specific patch isolation | Preserved via clean snapshot Draft PR #4 in fork |
| Production merge/release | BLOCKED |

**DMAIC:** Define the replay invariant; Measure current scope; Analyze
indistinguishable crashes and stale authentic state; Improve with smallest
versioned verifier; Control with regression/falsifiers, immutable anchors,
receipt readback and rollback. Cycle:
`ψ → χ → ρ → Δ → Σ → Ω → ψ′`.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`
`claim_allowed=false`

## Reproduction — host only

```sh
cd contrib/raf_freestanding_gate
cc -std=c11 -O2 -ffreestanding -fno-builtin -Wall -Wextra -Wpedantic \
   -Wconversion -Wsign-conversion -Werror -Iinclude \
   -c src/raf_gate_recovery.c -o /tmp/raf_gate_recovery.o
nm -u /tmp/raf_gate_recovery.o # expected none
cc -std=c11 -O1 -g -Wall -Wextra -Wpedantic -Wconversion \
   -Wsign-conversion -Werror -fsanitize=address,undefined \
   -fno-omit-frame-pointer -Iinclude \
   src/raf_gate.c src/raf_gate_recovery.c tests/test_raf_gate_recovery.c \
   -o /tmp/test_raf_gate_recovery
/tmp/test_raf_gate_recovery
```
