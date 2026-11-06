# RAFAELIA Ω · NUCLEOS — Evidence & Recovery Guard v0.1 (experimental)

An **independently licensed, freestanding C11 observer** placed beside a Firebird fork. It does not modify Firebird internals and it is not a Firebird feature.

**Implemented in v0.1:** bounded deterministic progress/clock/IO observation, explicit UNKNOWN state, adaptive sampling interval recommendation (bounded by policy), alert-only output, 48-byte public event frame with CRC32 for accidental-corruption detection, and a synthetic test harness.

**Not implemented:** Firebird adapters, process/CPU monitoring, I/O probes, OS integration, automatic remediation, authentication/signatures, backup/restore, live database runs, CI proof or independent audit. The kernel cannot detect deadlock from CPU percentage alone. An integration must supply *trusted monotonic* progress and time counters.

Safety: No library allocation, syscalls, threads, filesystem, networking, SQL, database writes, restarts, file restore or secrets. A failed sample stays UNKNOWN; no destructive action is ever emitted.

## Quick start (optional hosted tests)

```sh
cd NUCLEOS
sh tests/run.sh
```

The core is designed for C11 freestanding compilation. The harness intentionally uses a hosted C runtime for assertions and output; it is **not** a Firebird engine build. See [GATES](docs/GATES.md) and [THREAT MODEL](docs/THREAT_MODEL.md).

## Integration sequence

1. Validate on disposable test data without an adapter.
2. Build a separately licensed platform/Firebird read-only adapter after code review, using authorized Firebird Trace/MON$ sources.
3. Reproduce upstream [#9269](https://github.com/FirebirdSQL/firebird/issues/9269) on an isolated test Firebird (disk-full fault injection; no production storage).
4. Verify forward progress, no regression and data integrity before considering a minimal **separate** upstream engine fix.
5. Add *vetted* external authenticated receipt chain with key lifecycle, independent anchoring and documented privacy controls. CRC32 is **not** security.

Nothing is scheduled or started in the background.
