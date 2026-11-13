# Remote view and acceleration producers

Restores the original AFortPlayerPawn normal and replay replication lifecycle:
`PreReplication` (0x142344490), `PreReplicationForReplay` (0x142344770),
acceleration packing (0x1423442e0), remote view packing (0x142344420), and
slippery-tag predicate (0x14232caf0). The UFortAbilitySystemComponent accessor
0x1406ccd10 returns inherited RepAnimMontageInfo by const reference.

Normal replication calls its superclass, updates authority acceleration/view,
SkyTube/Water references and movement flags, then performs the all-role vortex
active override and ASC montage/landing copies. Replay calls its distinct
superclass path and packs only autonomous proxies. Montage position comparison
retains the original unordered-float behavior; NextSectionID changes still force
an update. FastSharedReplication is a separate, unrecovered path.

SSE intrinsics preserve the original float operations, MXCSR rounding and
indefinite conversions. The view scale is exact float bits 0x43360aab
(65535/360 rounded to float), with pitch in the high word and yaw in the low.
Do not replace this with UE angle compression or generic rounding. The slippery
predicate uses the existing shared native tag singleton container at +0x1b30.

The six entries are in behavior/MANIFEST.json. source_policy.py requires both
target classes, checks complete model signatures and bodies, verifies emitted
source, and injects x64 SIMD headers and Clang/GCC FP-contraction controls after
the own-header include. Architecture or fast-math mismatches fail generation or
compilation rather than changing arithmetic silently.

Evidence is retained under ignored generated/agents/remote-view-producer,
remote-view-lifecycle, and remote-view-producer-integration. Independent review
checked cached assembly, superclass implementations, prediction flags, ASC
accessor and native tag membership. Generation and the sixteen negative policy
cases passed. Numerical smoke coverage includes view packing with ordinary,
non-finite and out-of-range values; full acceleration differential and fresh
two-client replication acceptance remain pending.
