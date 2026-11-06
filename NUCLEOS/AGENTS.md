# AGENTS.md — NUCLEOS / RAFAELIA Evidence & Recovery Guard

Scope: only files below `NUCLEOS/`. Read [INDEX.md](INDEX.md) and [LICENSING.md](LICENSING.md) before changes. This directory is an **independent experimental component**, not Firebird upstream code, an installed plugin, a security certification or an autonomous recovery facility.

## 0. Mission and immutable invariants

Help humans and independent auditors reproduce observability, evidence and bounded fail-safe results without needing personal or production database data.

- Never change Firebird page/cache/transaction state here. No client cancellation, server restart, database restore, recovery automation or privileged process control.
- C kernel is deterministic, bounded, freestanding C11: no allocation, threads, libc calls, filesystem, clock, network, syscalls, dynamic loading or platform-specific ABI inside `src/`.
- Any future OS/Firebird integration lives in opt-in `adapters/` with a separate, reviewed contract. No adapter exists yet.
- Missing/invalid clocks, incomplete metrics, counter rewind and ambiguous observations produce `UNKNOWN` with **no destructive action**. Valid zero is different from missing data.
- Do not extrapolate diagnostic findings into data durability guarantees or claim hardware/runtime testing from isolated harness results.
- A CRC detects accidental corruption but **does not authenticate**. Do not call it a cryptographic hash, digital signature, immutable ledger or tamper-proof proof.
- No database records, full SQL, BLOB data, secrets, hostnames, user IDs or IP addresses in public event frames; also treat filenames/paths and per-instance stable identifiers as possible personal data.

## 1. Source and authority routes

| Area | Authoritative source | Allowed change |
| --- | --- | --- |
| Firebird issue #9269 | Firebird upstream report + verified exact HEAD | Evidence and minimal engine fix in a **separate branch** after repro |
| Firebird issue #9189 | Firebird upstream report + version-specific source | Separate investigation; no combined fix |
| `include/`, `src/` | NUCLEOS API v1, tests and separate MIT license | Deterministic standalone code only |
| `tests/` | Executable assertions over fixed synthetic inputs | Hosted dependencies permitted **only in tests** |
| `docs/`, `INDEX.md` | Explicit implementation/readback evidence | Proposals clearly marked NOT_IMPLEMENTED |
| GitHub fork | Implementation authority | Branch + SHA + independent readback |
| Drive/Supabase | Optional private index/receipt, **only if authorized** | Metadata-only append-only, no secrets |

`SOURCE -> PROVENANCE -> ARTIFACT -> EXECUTION -> EVIDENCE -> CLAIM`. Record each step separately. Unknown = typed `TOKEN_VAZIO`, not zero.

## 2. Contributing safely: exact sequence

1. DEFINE: state the issue, threat, CTQ, error budget and what **must not happen**.
2. MEASURE: read CURRENT_STATE when available, exact HEAD, latest upstream issue, existing PR, licenses; choose minimal roots.
3. ANALYZE: enumerate plausible causes and falsifiers, especially lost progress vs high CPU, clock discontinuities, counter wrap, disk-full during logging and concurrent data mutations.
4. IMPROVE: modify smallest independent API or code path on a feature branch; preserve public data format and ABI or introduce explicit version.
5. CONTROL: compile with `-std=c11 -ffreestanding -fno-builtin -Wall -Wextra -Werror -pedantic`; run hosted tests, fault injection and sanitizers when available; inspect diff and compare against base.
6. RECEIPT: record commands, platform, compiler version, exit codes, test scope, changed SHAs, limitations, and rollback.
7. R3: `<F_ok,F_gap,F_next>`; write one append-only delta to authorized ledger only if material and verifiable; never duplicate the corpus.

Never mark PASS on a code read, static assertion or different commit. Tests on x86_64 do not demonstrate ARMv7/ARM64 behavior or Firebird remote integration.

## 3. State machine and permitted actions

`UNKNOWN` (missing/invalid evidence), `HEALTHY`, `SUSPECT`, `STALLED`, `IO_ERROR`. The kernel's only possible decisions: `NONE`, `OBSERVE`, `ALERT`. Never auto-remediate.

A high CPU percentage by itself is **not** a stall. Progress requires a trustworthy monotonic counter. Inconclusive samples reset the observation baseline and preserve typed uncertainty. The watchdog must not cause a feedback loop or unbounded polling.

## 4. Event format and evidence

Versioned 48-byte public synthetic event frame described in [docs/FRAME_V1.md](docs/FRAME_V1.md). The CRC32 is a transmission-integrity check only. Any externally auditable receipt needs a **separately implemented** vetted cryptographic hash, domain separation, signature/checkpoint, independent anchoring and explicit key/retention policy. No such authentication is implemented in v1.

Before external sharing, validate consent, redaction, minimization and retention. Even hashed identifiers can reidentify users. Never publish production data or generated screenshots without review.

## 5. Gates and rollback

G0 deterministic falsifiers; G1 integrated exact-head Firebird build; G2 real remote and disk-full reproduction on disposable database; G3 data-integrity check; G4 privacy and secret scan; G5 signed/independently anchored receipts; G6 independent reproduction. `NOT_RUN` is not `PASS`. See [docs/GATES.md](docs/GATES.md).

Rollback: revert only the dedicated NUCLEOS commit/branch, or deploy a prior validated version of the standalone observer. Do not try to roll back database transactions, database files or upstream history. All recovery actuation remains out of scope.

## 6. Prohibited shortcuts

Do not rewrite licenses; copy Firebird engine source; fetch secrets; install invasive hooks; disable Firebird safety; expose operational topology; make unbounded memory use; automatically trigger multi-platform release jobs; publish a scientific claim of novelty; silently fill gaps; or merge without the repository owner's decision.

A scientific comparison against existing tracing/audit systems is required before any claims of world-first originality.

## 7. PR template minimum

State `intent | base_sha | head_sha | files | license scope | build commands | test matrix | PASS/FAIL/NOT_RUN | evidence ref | threat model | known gaps | claim_allowed | rollback`, then R3. Provide one issue per PR and link any upstream report without implying upstream endorsement.
