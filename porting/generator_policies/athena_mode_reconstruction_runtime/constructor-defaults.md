# Original constructor defaults needed by the callback

The original GameStateClass write is recovered. Constructor registration chain:
AFortGameModeAthena singleton 0x1428ee970 → InternalConstructor thunk
0x1428eebb0 → constructor **0x14192dab0–0x14192e915**. Its first constructor
call, at 0x14192dadd, is **AFortGamePvPBase constructor 0x14197f6d0**.
That parent identity is independently reached through its singleton
0x14290ac30 → thunk 0x142909920. It calls AFortGameModeZone constructor
0x141e89e00 at 0x14197f6e2. Original body extents and singleton providers are
pinned in constructor-defaults.json; validate_constructor_image checks them.

At **0x14192e503**, Athena calls **0x14290f690**, whose singleton-builder RDX
UTF16 name at 0x1463e7bda is **FortGameStateAthena**. At **0x14192e508** it
stores the returned UClass pointer to **+0x230**, the reflected inherited
GameStateClass property of AGameModeBase. The exact source operation is:

```cpp
GameStateClass = AFortGameStateAthena::StaticClass();
```

This closes the class-selection provider required by OnAircraftEnteredDropZone's
intentional AFortGameStateAthena cast. It does not establish runtime callback
execution or recover the remainder of the original constructor.

All ten direct Athena class-default writes, in original order:

| Field | Original offset | Original value class | StaticClass provider | Store |
|---|---|---|---|---|
| GameStateClass | 0x230 | AFortGameStateAthena | 0x14290f690 | 0x14192e508 |
| PlayerControllerClass | 0x238 | AFortPlayerControllerAthena | 0x142a53450 | 0x14192e514 |
| PlayerStateClass | 0x240 | AFortPlayerStateAthena | 0x142a8ae70 | 0x14192e520 |
| HUDClass | 0x248 | AFortUIPvP | 0x142b272c0 | 0x14192e52c |
| ServerBotManagerClass | 0x1040 | UFortServerBotManagerAthena | 0x142ad3a40 | 0x14192e538 |
| SpectateAFriendPlayerControllerClass | 0x1058 | AFortSpectateAFriendController | 0x142adf6d0 | 0x14192e544 |
| LiveBroadcastPlayerControllerClass | 0x1060 | AFortLiveBroadcastController | 0x142980fc0 | 0x14192e550 |
| ReplaySpectatorPlayerControllerClass | 0x260 | AFortReplaySpectatorAthena | 0x142ac7e20 | 0x14192e55c |
| MegaStormManagerClass | 0xfb8 | AMegaStormManager | 0x1429b4560 | 0x14192e568 |
| TeamInfoClass | 0x4e8 | AFortTeamInfoAthena | 0x142af1340 | 0x14192e58b |

Each value's original singleton name was checked independently, and each field
is mapped to its exact original reflected property record in the contract.
OriginalClassDefaults.inc is the corresponding **partial source fragment**.
There are non-class initializations between some stores and many constructor
effects before/after this fragment. Do not credit it as a recovered whole
constructor or silently omit those other effects.

Three inherited class defaults are relevant when preserving fixture behavior:
DefaultPawnClass is ADefaultPawn (provider 0x1446f5990, store 0x1441ffc65),
GameSessionClass is AGameSession (provider 0x1447174a0, store 0x1441ffca1),
both initialized by AGameModeBase inlined into original AGameMode constructor
0x1441ffb80. SpectatorClass is AFortReplaySpectatorPawnBase (provider
0x142acab80, store 0x141e44c96 in AFortGameMode constructor 0x141e44850).
Athena itself does not write DefaultPawnClass. The current fixture's
AFortPlayerPawnAthena assignment remains explicitly Linux runtime support;
this task does not change that established behavior.

## Optional owner-applied source correction

The migration script can add only the proven GameStateClass assignment and its
header while preserving every pre-existing fixture statement and other method.
Before applying the class migration, review this read-only plan:

```bash
python3 porting/behavior/athena_mode_reconstruction_runtime/migrate_fixture.py \
  --restore-original-game-state-class \
  --original-image /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin
```

Use the same command with `--apply` for the owner-controlled transition. The
flag requires successful original constructor/provider image guards; the
complete original manual .cpp checksum is also checked. This is an alternative
to the default byte-identical .cpp migration, performed in the same operation.
Do not run the default migration first and then this flag: repeat/partial
transitions intentionally fail closed. Preview of the proposed .cpp:
`/tmp/gameplay-athena-mode/FortniteAthenaGameplay.cpp.after-gamestate`.

The source fragment for all ten class stores is ready for a subsequent complete
constructor reconstruction. This narrow correction installs only the
GameStateClass selection needed by this callback; it leaves the other original
initialization effects explicit and open. Client/server/Apollo validation must
be included in the class migration's coherent integration batch. Original image
guards and the read-only correction plan passed; no Native source was changed
and no build/runtime callback was run here.
