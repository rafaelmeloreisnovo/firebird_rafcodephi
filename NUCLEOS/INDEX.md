# NUCLEOS — navigation / authority index

| Path | Role | Current status |
| --- | --- | --- |
| [AGENTS.md](AGENTS.md) | Scoped contributor/AI rules | WRITTEN |
| [LICENSING.md](LICENSING.md) + [LICENSE](LICENSE) | Boundary and independent license | WRITTEN |
| [include/raf_guard.h](include/raf_guard.h) | Freestanding observer API contract | IMPLEMENTED_UNTESTED until exact-HEAD run |
| [src/raf_guard.c](src/raf_guard.c) | Pure deterministic watchdog and CRC event encoder | IMPLEMENTED_UNTESTED until exact-HEAD run |
| [tests/test_guard.c](tests/test_guard.c) | Synthetic positive/adversarial behavior tests | PROVIDED; execution evidence separate |
| [tests/run.sh](tests/run.sh) | Offline local-only build/test entrypoint | PROVIDED; requires C11 compiler |
| [docs/FRAME_V1.md](docs/FRAME_V1.md) | Canonical event frame, privacy and cryptographic limits | SPECIFIED |
| [docs/GATES.md](docs/GATES.md) | Acceptance and non-claims | SPECIFIED |
| [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) | Failure modes and boundaries | SPECIFIED |
| [ROADMAP.md](ROADMAP.md) | Scoped #9269 and #9189 next steps | PLANNED |

No engine code was modified by this subtree. No adapter, runtime instrumentation, cryptographic signature, ledger or active recovery is implied by the folder name. Follow project-level [AGENTS.md](../AGENTS.md).
