# C: classes, Blueprint defaults, abilities and the world on the server

Date: 2026-11-11. Facts are read from the 12.60 content with the asset tools
(`porting/tools/README.md`). The level-streaming and class-loading parts are
reasoning, not yet tested; they say what to check first.

## 1. Blueprint classes the match runs on (facts)

`/Game/Athena/Athena_GameMode` (`Athena_GameMode_C`) class defaults:

| Property | Value |
|---|---|
| GameStateClass | `Athena_GameState_C` |
| PlayerControllerClass | `Athena_PlayerController_C` |
| SpectatorClass | `BP_SpectatorPawn_C` |
| SafeZoneIndicatorClass | `SafeZoneIndicator_C` |
| GE_OutsideSafeZone | `GE_OutsideSafeZoneDamage_C` |
| MegaStormManagerClass | `MegaStormManagerAthena_C` |
| ServerBotManagerClass | None (bots are created another way; D15) |
| CoreGameModeModifiers | `DriftboardSwapModifier` |
| DeathPenaltyData | `Anthem_DeathPenalty` |
| **StartingItems** | 1 × `BuildingItemData_Wall`, `BuildingItemData_Floor`, `BuildingItemData_Stair_W`, `BuildingItemData_RoofS`, 1 × `EditTool`, 0 × `WoodItemData` |
| bOverrideQuickBars | true |
| QuickBarDefinitions[0] (primary) | slot 0: WeaponHarvest/AthenaGadget (pickaxe); slots 1–5: WeaponMelee, WeaponRanged, Gadget, Consumable, AthenaGadget |
| QuickBarDefinitions[1] (secondary) | 4 static BuildingPiece slots, then 3 Trap slots |
| storm cap / camping settings | curve handles into `AthenaGameData` (`Default.SafeZone.*`) |

So the spawn inventory is: the StartingItems above **plus the pickaxe from the
player's cosmetic loadout** (not in StartingItems) **plus stone and metal at 0**
if the client HUD needs the entries (check on the first inventory test: if the
stone/metal counters are missing, add them).

`Athena_GameState_C`: `BuildingActorClasses` (the full list of player-buildable
`PBWA_*` classes: the server validates `ServerCreateBuildingActor` class
against it), fast arrays `GameMemberInfoArray`, `CurrentPlaylistInfo`,
`MutatorObjectDataArray`, `SpawnMachineRepData`, `ActiveGameplayModifiers`;
`VolumeManagerToUse = FortVolumeManager_BP_C`.

`Athena_PlayerController_C`: components `MarkerComponent`,
`ResurrectionComponent`, `HeldDeviceUsageComponent`, `XPComponent`; three
quickbar definitions (primary, secondary, creative); quick-edit patterns.

`PlayerPawn_Athena_C`: cosmetic and storm-visual settings only; gameplay values
come from the native parent and attribute sets.

Default pawn class: not a GameMode Blueprint default; `SpawnDefaultPawnFor`
(0x141967ce0) takes it from the playlist/game data ("no default pawn class"
error path). Read that function in full when porting.

## 2. Default player abilities (facts)

`/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer`
(`FortAbilitySet`):

- Abilities: `FortGameplayAbility_Jump`, `FortGameplayAbility_Sprint`,
  `GA_DefaultPlayer_Death_C`, `GA_DefaultPlayer_InteractUse_C`,
  `GA_DefaultPlayer_InteractSearch_C`, `GAB_AthenaDBNO_C`,
  `GAB_AthenaDBNORevive_C`, `GA_AthenaEnterVehicle_C`,
  `GA_AthenaExitVehicle_C`, `GA_AthenaInVehicle_C`,
  `GA_Rift_Athena_Skydive_C`, `GA_Athena_ZipLine_Smash_C`,
  `GAB_CarryPlayer_C`/`DropPlayer`/`ThrowPlayer`, hiding-prop, door-slam,
  interrogate, pet, slurp, siphon and a few passives (31 total).
- Effects: `GE_DefaultResourceCap_C`, `GE_DefaultTurboBuildSettings_C`.

Server duty: after the pawn is possessed, give every ability in this set to
the player's ability system component (on the PlayerState) and apply the two
effects. `UFortAbilitySet` has a grant function; check it with `rpc_sweep.py
GiveToAbilitySystem`. If empty: loop `GiveAbility(FGameplayAbilitySpec(Class,
1))` and `ApplyGameplayEffectToSelf`. Weapon abilities are granted when a
weapon is equipped (the weapon definition lists them); that is inside
`EquipWeaponDefinition` (real).

**Consequence:** jumping, sprinting, interacting (chests, doors), entering
vehicles and dying are all ability Blueprints that must *load and run on the
server*. This is the strongest reason C1 (Blueprint loading) comes before any
gameplay system: without the ability Blueprints the player cannot even open a
chest. Death is `GA_DefaultPlayer_Death_C`: read its graph
(`Cue4Inspect … --function-report`) before writing the death sequence in the
D6 card; part of that sequence may already be done by the ability.

## 3. C1: loading Blueprint classes on the server (reasoning)

Known from the frontend work: a cooked Blueprint loads only if every native
class in its parent chain exists with the right properties **and** every
native function it overrides or calls is declared. Order to try, smallest
dependency set first:

1. `Playlist_DefaultSolo` (data asset; needs `UFortPlaylistAthena` and the
   table classes). Already parses outside the engine.
2. `AthenaGameData` and the two loot tables (need the row structs
   `FortLootTierData`, `FortLootPackageData`: check the generated structs match
   the fields listed in the D4 card §5).
3. `Athena_GameState_C`, then `Athena_GameMode_C`.
4. `Athena_PlayerController_C`, `PlayerPawn_Athena_C` (largest closure: pulls
   cosmetics, sounds, UI classes by hard reference).
5. `GAS_AthenaPlayer` and its 31 abilities.

For each: `Cue4Inspect <asset> --imports-json --loose` gives the hard imports;
list the `/Script/FortniteGame` classes and functions it names; compare with
the generated native class set; add what is missing; load; read the log for
`Skipping import`, `Could not find`, `RF_NeedLoad`. On a dedicated server,
client-only imports (widgets, sounds, materials) may be left unresolved if the
load does not assert; record each case instead of fixing it.

Fallback if a Blueprint cannot be made to load in reasonable time: spawn the
native parent class and set the listed defaults in C++ from this card. This
works for GameMode and GameState (few defaults). It does not work for
abilities (logic is in the graph).

## 4. C2: the world (reasoning and what to check)

What the server needs from the map: collision (landscape, buildings), player
starts on the warmup island, building actors (they are replicated by static
path, the client resolves them in its own copy of the level), containers and
floor-loot spawners, vehicle spawners, volumes.

What the client needs from the server: the same level names in the same
streaming state, and for every replicated level actor a path that exists on
the client. The client loads `Apollo_Terrain` and streams sublevels itself by
distance (world composition); the server on a dedicated build loads all of
them at start. A replicated actor in a sublevel the client has not streamed in
is simply not relevant yet; that is normal engine behaviour.

Checks, in order:

1. Does `Apollo_Terrain` open on the server with world composition, and how
   many of the 87 sublevels load? (`streamed_levels_inventory.md`.)
2. Are building actors in sublevels net-startup actors with stable names on
   both sides? Take one chest, compare its full path in the client's level
   (asset tools: `--actor-report`) and on the server.
3. Playlist levels: `AdditionalLevels` (one for solo) must be streamed on both
   sides; the GameState replicates the list.
4. Until the full map loads: use a **small test map path that exists in the
   client's paks** for G5/G6 (the client must have the same package). The
   warmup island is part of `Apollo_Terrain`, so there is no smaller official
   map with the same game mode; so this fallback means running the Athena game
   mode on `Apollo_Terrain` with only the persistent level and the sublevels
   that load, and accepting missing collision elsewhere.

## 5. Open (not thought through further)

- Vehicles (D14): `AFortAthenaVehicleSpawner::SpawnVehicle` is empty (D5
  card); vehicle classes have real `ServerOnAttemptInteract`,
  seat and exit RPCs. Physics pawns (`AFortPhysicsPawn::ServerMove`,
  `ServerUpdateStateSync`) are **empty**: vehicle movement replication on the
  server must be re-created. Leave vehicles out of the first playable target.
- Bots and henchmen (D15): bot manager located; `ServerBotManagerClass` is
  None in the game mode; `AAIHotSpot` natives are empty. Leave out.
- Quests: client-side progress display works from profile data; server-side
  quest progress is not looked at.
