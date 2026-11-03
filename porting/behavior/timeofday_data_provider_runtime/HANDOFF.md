# Time-of-day provider source recovery

Corrected binding: the data owner is `UGameDataSTW`, not `UFortGameData`.
0x142b60fa0 is `UGameDataSTW::StaticClass`: its static registration names
`GameDataSTW`. Reflection independently establishes UFortAssetManager's
`GameDataSTW` at +0x500, `GameDataNameSTW` at +0x508, and UGameDataSTW's
`FortTimeOfDayData` at +0x10d8. The earlier proposed native field must not be
integrated; the existing reflected field is the real owner.

0x1424e1120 calls the existing engine asset-manager accessor and then
0x141df78c0. The latter returns the cached STW data when nonnull, otherwise
calls 0x141df76b0 using UGameDataSTW class name as the FPrimaryAssetType.
The latter tests the configured STW name for emptiness; it calls the virtual
+0x310 with the one-FName type, `DefaultBundleState` (+0x498), an empty
delegate and zero priority. The local UE header identifies the matching API as
`LoadPrimaryAssetsWithType`. For a valid streamable handle it waits with
`(0.0f, false)`, obtains the first loaded asset and casts to
UPrimaryDataAsset. It destroys the handle before failure handling. Failure
logs fatal with the configured name and type; the wrapper then casts the
result to UGameDataSTW and caches it, including null for a wrong subclass.
The timer/progress/stat scopes have no gameplay effect here. The proposal uses
the actual reflected fields and existing manager accessor, without parallel
state or a cache-only fallback.

0x1424e4940 passes `FortTimeOfDayData` and `true` to 0x1424ce5d0. The typed
resolver exits without tracking for a null path, otherwise checks the cached
object, invokes delayed loading if needed, type checks as UFortTimeOfDayData,
and tracks the resulting UObject pointer, including null.

Both AFortGameMode and AFortGameModeAthena vtable +0x980 point to 0x1419458e0,
which returns `this + 0x4b0`, the reflected `GameContextTags` member. The
base virtual declaration also covers Athena without an override.

Evidence: original image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base 0x140000000; probes `/tmp/tod-probe`, `/tmp/tod-probe2`, `/tmp/tod-common-probes`, and `porting/generated/callee_probe/gameplay_rough_pass_1`. Base vtable 0x145fedcb0 and Athena 0x145dfb470 have 0x1419458e0 at +0x980. No tests, build, or runtime were run.
