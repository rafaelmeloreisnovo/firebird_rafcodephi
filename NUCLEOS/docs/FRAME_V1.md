# Event frame v1 — 48 bytes, public synthetic-only

Frame is canonical little-endian, fixed 48 bytes, encoded by `raf_frame_encode`.

| Offset | Size | Meaning |
| --- | ---: | --- |
| 0 | 4 | ASCII `RAFG` |
| 4 | 1 | Version = 1 |
| 5 | 1 | Known-field bits (1 tick, 2 progress, 4 IO) |
| 6 | 1 | State enum (0 UNKNOWN, 1 HEALTHY, 2 SUSPECT, 3 STALLED, 4 IO_ERROR) |
| 7 | 1 | Action enum (0 NONE, 1 OBSERVE, 2 ALERT) |
| 8 | 8 | Observer-local sequence |
| 16 | 8 | Monotonic tick, zero if unknown |
| 24 | 8 | Progress counter, zero if unknown |
| 32 | 8 | Recommended polling interval |
| 40 | 1 | Reason enum |
| 41 | 3 | Reserved, MUST equal zero |
| 44 | 4 | CRC32/IEEE reflected over bytes 0–43 |

The `known` bits distinguish **valid zero** from **unknown**, even though unknown fields are serialized as zero. Tick units and observer-counter semantics are provided by a future adapter; this format itself stores no wall-clock date, user, hostname, SQL or raw payload.

CRC32 detects accidental corruption only: it is trivial to forge and cannot establish provenance, order across independent streams, non-repudiation, privacy, authenticity or data integrity of the Firebird database. Do not label it a signature, cryptographic digest or public audit receipt. A verified public audit chain would require vetted cryptography and external anchored checkpoints; **NOT_IMPLEMENTED**.

Frame validation rejects bad magic/version/CRC/reserved bits/unknown enums. It does not validate source-trust or imply the event describes a real Firebird execution.
