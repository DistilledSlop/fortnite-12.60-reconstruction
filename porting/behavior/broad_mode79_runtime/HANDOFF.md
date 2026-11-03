# Broad mode 79

Two uncounted, unvalidated safe-zone dependency lifts: `0x14199e430` and
`0x14199e4e0`, with source paths and SHA-256 in `sources.json`. Both reload the
selected override/base playlist between predicate calls; the second also tests
a global enable byte, two playlist name arrays and playlist byte 0x467. Exact
object access, name predicate, global getters, playlist predicates, null behavior,
array lifetime/storage and calling conventions remain unresolved. Pointer-array
arithmetic is descriptive and requires proper UE container/lifetime integration.
No compiler, test, build, generation, native promotion, staging or commit run.

The five candidate callbacks were already drafted: SafeZoneInsideChecks in
`gamemode_broad69_runtime`, CosmeticUsage in `gamestate_broad72_runtime`,
CurrentPlaylistInfo and PlaylistEndTime in `gamestate_broad66_runtime`, and
ReticulatedSplineDefinitions in `gamestate_callbacks67_runtime`. The latter
are rough drafts, not evidence of verified or complete native implementations.
These helper bodies were found in `callee_probe/blocking` and existing behavior
references to them were calls rather than body definitions.
