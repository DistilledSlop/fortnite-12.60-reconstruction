# Time-of-day manager selection recovery

The refresh fragment is source recovery only, not integrated. Its required
`AFortWorldSettings::NativeSpawnTimeOfDayManager()` implementation is not yet
closed. Both methods return `AFortTimeOfDayManager*`: refresh tail-jumps to
spawn when its field is null, and otherwise returns spawn's RAX after storing
it. `Destroy(false, true)` can reenter; do not clear the field before it or
write the null-branch result to it. The first branch's spawn itself may write
the field. Return ABI is explicit in original assembly even though Ghidra
misreports spawn as void.

Spawn gates in order: role byte == 3; GetWorld nonnull; world byte +0x11b
bit 0x40; `UWorld::IsGameWorld()` (0x141f6fca0); bSpawnTimeOfDayManager;
`IsInPersistentLevel(false)` (0x143f7ad10); world type != 7. On failure it
returns null without touching the manager. Then an ordinary
`TActorIterator<AFortTimeOfDayManager>` uses flags 5 (OnlyActiveLevels and
SkipPendingKill). For the first suitable actor, it stores the actor only if
TimeOfDayManager is null and returns the resulting field. This difference is
observable with a preexisting field. The iterator's destruction must run.

Without an existing suitable actor, 0x1416412b0 casts the world's authority
game mode to AFortGameMode. Its virtual +0x930 supplies a class; for Athena,
0x141949030 first returns the checked gameplay override, otherwise gets the
current Athena playlist and resolves its TimeOfDayManager soft class using
0x14192c570. Preserve dynamic dispatch: other game modes need not behave the
same. A null or wrong-base class enters the theme path. The other virtual
+0x980 returns the game context tags; the Athena target 0x1419458e0 returns
`&GameContextTags`. When 0x141ed8de0(0,0) succeeds, the game-data provider
0x1424e1120 and delayed asset resolver 0x1424e4940 obtain
UFortTimeOfDayData; its WorldAmbientPropertiesList is traversed in order until
its gameplay tag query matches. The matching TimeOfDayTheme's helper
0x141f60280 repopulates ValidTimeOfDayManagers from collections and additional
classes, removes prohibited classes when the valid list is initially empty,
then chooses a random soft class; no match or empty path yields null. The
implementation of rebuilding 0x141f5ba50 and its dependency 0x141f4fed0
needs full source closure before substituting a naive list selection.

If the selected class has no class default object, the original calls class
virtual +0x390 to create one, checks again, and falls back in this order:
MissionTimeOfDayManager, ZoneThemeTimeOfDayManager, WorldTimeOfDayManager,
DefaultWorldTimeOfDayManager. Each uses 0x14192c570 and each intermediate
candidate gets the same default-object test. The last candidate receives the
final common test. A usable class is checked as a child of
AFortTimeOfDayManager, and spawned by `UWorld::SpawnActor` at null location and
rotation with Owner = settings, object flags 0x40, and spawn parameter byte
+0x28 set to 1. The exact public parameter corresponding to that byte must be
verified against the matching engine version before final code. The spawn
result is stored to TimeOfDayManager, then the original calls shipping empty
stub 0x1406801c0 with bShowTimeOfDayManager if nonnull, and reloads the field
before return.

The typed resolver 0x14192c570 cannot be replaced with LoadSynchronous alone:
for every nonempty soft path it tracks the resolved typed class or null via
FFortAssets 0x141df61e0/0x141de9e00. See
`../item_ammo_definition_runtime/HANDOFF.md` and
`../fort_assets_retention_runtime/HANDOFF.md`: shared asset retention lifetime
and callbacks remain unintegrated. Theme data loading 0x1424ce5d0 likewise
tracks the result. `0x141ed8de0(0,0)` is a game-context predicate and still
needs its exact provider semantics. These dependencies prohibit a faithful
standalone spawn implementation or canonical manifest entry today. No stub or
silent fallback is supplied.

Evidence: original late-capture image
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
base 0x140000000. SHA-256 of exact contiguous windows: spawn 0x141fbd1f0,
0x68b bytes, c98974153b5070ba6f55eef7913bf55e3fc841939f189956249161293ba3a27d;
refresh 0x141fd72d0, 0x41 bytes,
fe173e65dd43d6d9cca693e7f17e33285f38430f012ad58555ffa87dbf37005b;
Athena +0x930 target 0x141949030, 0xd4 bytes,
069d048ff65e0ec41b70f4067cb8ad527d6583beb7fd52d13b08e4dbeba9b8fe.
Original probes are under `porting/generated/callee_probe/aircraft_larger_recovery{,2}`;
additional read-only probes were written to `/tmp/tod-probe` and
`/tmp/tod-probe2`. No build or tests were run.
