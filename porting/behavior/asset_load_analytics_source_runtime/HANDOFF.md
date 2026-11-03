# Promoted analytics source (batch 38)

Canonical fragments and manifest entries now implement all four emitters, Athena/Core
attribute builders, zone values and primary mission lookup. Native provider binding
is in controller_analytics_provider_binding_runtime; mission and match-time virtuals
are in analytics_mission_time_virtuals_runtime. UWorld::GetMapName is verified against
original14468f490 and Engine/Private/World.cpp4771. The older AssetLoadAnalytics.cpp/.h
namespace proposal is not emitted; use the canonical manifest helpers. Root corrected
provider slot to TSharedPtr<FFortNativeAnalyticsProvider,NotThreadSafe>& and preserved raw Athena
provider capture versus Core shared-slot reload after attribute construction. Attribute
arrays reserve1 initially and Num+3/Num+4 before appending; original order is retained.

Original full probe instruction ranges, including chained unwind fragments, are pinned
in native-evidence.json and checked during generation. Provider assignment and full
tracker callback/owner wiring remain separate unported work. No builds/tests run.

## Earlier bounded findings

# Asset load analytics source recovery

`AssetLoadAnalytics.cpp` and `.h` provide source-level proposed support for the four emitters and two attribute builders. `AthenaPhase141f90650` is independently closed; all other bodies have explicit, unresolved integration contracts declared in the header. They are not ready to link or promote until those contracts are implemented with the original dispatch and ownership semantics, and compilation and behavior are verified. `manifest-entries.json` proposes the independently closed `AFortMissionManager::NativeFindPrimaryMission` for 0x1420bc640. The phase conversion is standalone support source rather than a class method, so it has no class manifest entry. Names are assigned address-based integration names, not original symbol claims. No tests or build were run.

The provider contract must return the original controller +0x5a0 interface's +0x28 result slot by reference, and preserve its lifetime. Athena captures the provider before world access and attribute generation; Core checks the slot then reloads it at event dispatch. Original provider virtual +0xc8 records the event. File is first, followed by AthenaGamePhase (string), AthenaMatchTimeSec (double converted from float), AthenaMapName (string), or NumberPlayers (double converted from int32), ZoneName (string), GameDifficulty (double converted from float), MissionName (string). Core only adds those for FortGameStateZone excluding FortGameStateAthena; Athena only for FortGameStateAthena. Attribute helpers leave an existing array intact before appending. Null controller or null provider skips emission; event names and ordering are in the source.

`0x141e9a800` counts non-null PlayerArray entries with neither bit 1 nor 2 set at player state +0x222, reads Zone GameDifficulty +0xcc4, reads the name of the object at +0xb78 or `<Unavailable>` if absent, and initializes mission to `NullMissionManager`. If +0x3b0 exists and 0x1420bc640 returns nonnull, its virtual +0x690 and 0x142c40540 supply the replacement mission string. Exact field types, virtual meaning, and shared-return lifetime remain unclosed; do not implement the contract as a fixed value. Map helper 0x14468f490 iterates world +0x98, finds the first object of class 0x144782370, gets its name via 0x1442dbfb0, falls back to the world package name if empty, then transforms via 0x142d2d320. Its exact APIs remain unclosed. Athena game-state virtual +0x780 is also not named; preserve float return and virtual dispatch. The phase conversion returns None, Setup, Warmup, Aircraft, SafeZones, EndGame, or Unknown for every other byte.

Evidence: original image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base 0x140000000, SHA-256 338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680. Existing probes `porting/generated/callee_probe/fort_assets_diagnostics{,4}/`; four new probes in ignored `porting/generated/callee_probe/asset_load_analytics/`, targets in this directory. Probe transaction rolled back; project metadata changed. Source is a reviewable proposal, not a replacement for the outstanding contracts. No canonical manifest, native source, logs, counts, build, tests, or commits changed.

Follow-up: the new `FindPrimaryMission.inc` implements 0x1420bc640. Reflection identifies manager +0x260 as `Missions`, mission +0x260 as `MissionInfo`, and info +0x58 as `MissionType`, whose zero value is `Primary`. The zone helper now has concrete source: +0x3b0 is `MissionManager`; +0xb78 is `ZoneTheme`; player bits are `bIsSpectator` and `bOnlySpectator`. Its remaining dependency is the mission virtual +0x690 returning FText; it must retain dispatch and may not be substituted with a guessed field. `GetName` corresponds to FName-to-string conversion 0x142ce9f90. New probe `asset_load_analytics2` confirms first-primary selection and the map helper's class is LevelStreamingPersistent. Provider +0x5a0 remains an unidentified native interface and no claim is made that its returned shared slot is a raw-pointer reference in the final ABI.
