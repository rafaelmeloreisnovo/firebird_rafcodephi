# Gates / evidence levels — initial declaration

| Gate | Evidence required | Initial state |
| --- | --- | --- |
| G0 | Freestanding C11 build + exact-head synthetic adversarial tests | NOT_RUN until build receipt |
| G1 | Firebird exact-head build, upstream test suite and ABI compatibility | NOT_RUN |
| G2 | Repro upstream #9269 using disposable limited-size volume with remote clients | NOT_RUN |
| G3 | Post-fault Firebird integrity check on **copy**, no committed-data loss | NOT_RUN |
| G4 | No secrets/PII in public test frames, code and artifacts; independent review | NOT_RUN |
| G5 | Cryptographically authenticated receipts, key lifecycle and independently anchored checkpoint | NOT_IMPLEMENTED |
| G6 | Independent reproduction across declared OS/architectures | NOT_RUN |

Operational rule: no automatic failover, no production deployment, no release, no merge to upstream from this directory. Partial tests may be PASS_SCOPED but cannot upgrade G1–G6.

For any receipt record: `event_id | parent | source_ref | base_sha | head_sha | changed_files | compiler | command | platform | observed_result | negative_tests | evidence_hash | limitations | rollback | claim_allowed`.

Use `TOKEN_VAZIO` for unobservable evidence, not `PASS`. A recorded test invocation must include actual exit code and exact commit SHA.
