# Playlist mode predicate bindings

These fragments reproduce already-ported canonical bodies in `AFortGameStateAthena`;
`MANIFEST.json` is a local snapshot, not an additional set of registrations. The
reflected `IsCreativeOrPlaygroundModePlaylist` exec at `0x14291fec0` calls native
`0x14199e430` directly. The reflected function is Final and its generated C++
declaration is nonvirtual. Other recovered native callers can use that method;
the two null-safe playlist predicates are static helpers, not playlist members.

The guard selects override then base playlist separately for each predicate;
short-circuiting prevents the second selection after a Creative match. The
shared `FFortNativeGameplayTagsRuntime::Get()` owns the original `0x141ed1800`
lazy singleton registration sequence. Its registered `Athena.Creative` and
`Athena.Playground` fields correspond to original offsets `0x1748` and `0x1768`.
`UFortPlaylist.GameplayTagContainer` is at `0xd0` in reflected original layout;
its explicit and parent tag arrays occupy `0xd0` and `0xe0`. `HasTag` preserves
the invalid-tag check and both searches. Creative here is tag-only, without the
separate CreativeMode enable-bit gate. See `../playlist_mode_evidence.md` and
`../native_gameplay_tags_runtime/README.md` for the registration and reflection
evidence. Never cache the selected playlist across the two helper calls: lazy
registration can reenter and change it.

Raw windows from `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, image base `0x140000000`:

| Native body | Bytes | SHA-256 |
| --- | ---: | --- |
| `0x14199e430` | `0x4f` | `d9cde5528ce8309be37efc9d9a6bb3d6c5e2b32ac38ed6aaa341b27c25de0f11` |
| `0x141a49590` | `0xdc` | `f6c479946685b481e510682b2b0f8b500602938ef099e6cf20caf63b8734b185` |
| `0x141a4b0d0` | `0xdc` | `12c90b70ad6b30988979c726a46f692751d938638afe556b44717bf417946a1f` |

The raw decompilations are in ignored `porting/generated/callee_probe/aircraft_larger_recovery/` and `aircraft_larger_recovery2/`. These are static observations; no build or runtime test was performed in this bounded investigation.
