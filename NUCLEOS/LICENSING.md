# License and provenance boundaries

**NUCLEOS is separately licensed under [MIT](LICENSE), limited to original files written for `NUCLEOS/`.** Source headers use `SPDX-License-Identifier: MIT`. The root `AGENTS.md` is contributor guidance, not an attempt to relicense the Firebird tree.

The Firebird source and derivatives in `src/`, `builds/`, `examples/`, `extern/`, etc. remain under their applicable upstream licenses, including InterBase Public License (IPL), Initial Developer's Public License (IDPL), and third-party license notices. Nothing in this directory purports to replace, supersede, waive or relicense them.

| Component | Scope | License/status |
| --- | --- | --- |
| `NUCLEOS/include/`, `NUCLEOS/src/` | Clean-room original watchdog/event code | MIT |
| `NUCLEOS/tests/` | Original synthetic harness/scripts | MIT |
| `NUCLEOS/docs/` + `NUCLEOS/*.md` | Original NUCLEOS documentation | MIT |
| Firebird upstream engine and bundled dependencies | External inherited works | Existing upstream/third-party licenses: unchanged |
| Future external adapters | Not created; independent review needed | TOKEN_VAZIO / undecided |
| Project name/logo/certification/patents | No additional rights granted by this scope statement | Separate determination required |

**Provenance requirements:** If future code includes upstream/third-party portions, preserve every original notice, record exact origin/commit and its license, and resolve compatibility before distribution. Do not label such copied code MIT merely because it resides inside `NUCLEOS/`.

MIT is a permissive license for these original files, not a warranty of originality, patent clearance, security, interoperability or upstream acceptance. Legal analysis and contributor assent remain separate governance gates.
