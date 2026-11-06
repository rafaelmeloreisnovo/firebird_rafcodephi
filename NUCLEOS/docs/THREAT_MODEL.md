# Threat model / reversibility

## Protected assets
Production data integrity, transaction isolation, secrets, source provenance, predictable recovery, Firebird uptime, independent auditor trust.

## In-scope failures (synthetic only in v0.1)
Missing metrics vs valid zero, reordered/repeated progress, non-monotonic time, sequence exhaustion, disk-full observation, stale progress, malformed event, bit flips, malformed configuration.

## Security boundaries
- The observer is **read-only by construction**: outputs are state, reason, action NONE/OBSERVE/ALERT and interval recommendation. It has zero Firebird/OS hooks.
- Input counters are untrusted unless independently authenticated by an adapter. Stalled progress `!=` server deadlock.
- The frame format never accepts application payload or rows. Counter values still may be sensitive operational metadata: minimize, rotate identifiers, enforce retention.
- No self-healing, restarting, database restore, modifying cache, touching backup or scrubbing Firebird logs.
- CRC32 is not security. Compromised observer, forged telemetry, compromised keys, wall-clock substitution, disk-full while persisting evidence, and false recovery remain unmitigated by v0.1.

## Verification
A future guarded adapter must be separate, have read-only credentials, bounded sampling and hard resource limits, and fail without blocking the DB. On error: UNKNOWN, no remediation. External audit claims require an independently reviewed chain, threat-tested redaction, a recovery drill on disposable volumes and a published test manifest.

## Rollback
Stop/disable the optional observer (when an adapter eventually exists) or revert the feature branch/commit; Firebird engine state and production database contents remain untouched. Database transactional rollback/recovery is an independent operator-controlled process.
