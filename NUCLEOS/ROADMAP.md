# Issue-scoped roadmap, not a scheduled operation

**Baseline:** Firebird upstream master at the commit recorded by the independent development branch. This NUCLEOS commit contains **only new paths**. No engine patch for #9269 or #9189 is implemented here.

| Checkpoint | Next reversible step | Acceptance |
| --- | --- | --- |
| C0 | Compile and run freestanding guard tests (G0) | Recorded exact-head test receipt |
| C1 | Create a separate isolated Firebird read-only adapter, if approved | No DB writes, secrets or unbounded polling |
| C2 | Reproduce [#9269](https://github.com/FirebirdSQL/firebird/issues/9269) on a disposable disk-full test volume | Baseline failure reproducible, explicit timeout, full evidence and restart/recovery plan |
| C3 | Locate actual root cause and minimal engine patch in separate branch | Differential test passes; counterexamples fail original |
| C4 | Repeat [#9189](https://github.com/FirebirdSQL/firebird/issues/9189) separately, avoid assuming same root cause | No cross-issue regression |
| C5 | Security audit of authentication, redaction, provenance and external anchoring | G4/G5/G6 actually run and documented |

Decision points require human authorization and provider-controlled gates. `claim_allowed=false`, `upstream_ready=false` until integrated CI, privacy, independent review and relevant reproduction.
