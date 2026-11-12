# D4: game data that is already in the client content (playlist, storm, bus, loot, balance)

Date: 2026-11-11. Read with the inspector as described in
`porting/tools/README.md` ("Game data from the extracted content"). Values below
are copied from the 12.60 assets; they are facts, not guesses.

**Main conclusion: the numbers that drive a Battle Royale match are data, and
the data shipped with the client.** Storm sizes and times, bus height and speed,
supply drops, DBNO, harvesting rates, fall damage, XP per accolade, and the
complete loot tables with weights are all present. The server should load the
same assets (it cooks/loads the same paks) and read these tables through the
ported code. Nothing in this card needs to be invented.

## 1. Solo playlist: `/Game/Athena/Playlists/Playlist_DefaultSolo`

| Property | Value |
|---|---|
| PlaylistName / PlaylistId | Playlist_DefaultSolo / 2 |
| MaxPlayers, MaxHumanAndBotParticipants | 100, 100 (bOverrideMaxPlayers) |
| MaxTeamCount / MaxTeamSize / MaxSquadSize / MaxSocialPartySize | 100 / 1 / 1 / 1 |
| WinConditionType | MutatorControlledChinaSupported (last team standing in practice) |
| GameData | `/Game/Athena/Playlists/AthenaCompositeGameData` = AthenaGameData + OverrideGameData |
| LootTierData | `/Game/Athena/Playlists/AthenaCompositeLTD` = `Items/Datatables/AthenaLootTierData_Client` + OverrideLootTierData |
| LootPackages | `/Game/Athena/Playlists/AthenaCompositeLP` = `Items/Datatables/AthenaLootPackages_Client` + OverrideLootPackagesData |
| LootLevel | 1 |
| SafeZoneLocationBlacklist | `/Game/Athena/Balance/DataTables/AthenaSafeZoneBlacklist` |
| PlaylistMissionGen | MissionGen_Athena_C |
| AISettings | Athena_AI_Settings; BotVersionPlaylistName Playlist_Bots_DefaultSolo; bEnableDynamicBotBackfill true |
| ModifierList | MaterialDropOnElim_Default, ContextTutorial_AthenaContextTutorial, GameplayMod_Phoebe_DM_Solo, GameplayMod_DeimosAI |
| AdditionalLevels (client and server) | `/Game/Athena/AI/MANG/HACK_MangMusicLevel` |
| AdditionalLevelsServerOnly | the `*_MANG` levels (Agency, OilRig2, SJI, Yacht, MountainBase and several small POIs): henchmen/boss spawners |
| ItemsToFullyLoad | boss weapons (WID_Boss_Midas, …), AGID_Boss_Tina, WID_Pistol_Donut |
| GameplayTagContainer | Athena.Playlist.DefaultXP, .Default, .Core, .Solo |

Decisions that follow:

- First playable target uses this playlist with **bots and AI modifiers off**
  (skip GameplayMod_Phoebe_DM_Solo and GameplayMod_DeimosAI, skip
  AdditionalLevelsServerOnly). The client does not need them. They come back
  with D15 (AI).
- `AdditionalLevels` must be streamed and the list replicated on the GameState
  (`AdditionalPlaylistLevelsStreamed`), or the client and server level sets
  differ. One level only, so low risk.
- The playlist object itself must replicate to the client through
  `CurrentPlaylistInfo` on the GameState (fast-array struct with BasePlaylist,
  OverridePlaylist, PlaylistReplicationKey). The client's HUD, team size rules
  and `LoadCurrentPlaylistData` (0x14199f7df) run from it.

## 2. Storm (safe zone): `AthenaGameData`, rows `Default.SafeZone.*`

Curves are indexed by safe-zone phase number (x) and give the value (y). Linear
interpolation, but only integer phases are used.

| Row | Phase → value |
|---|---|
| Count | 10 phases |
| StartDelay | 60 s after the bus phase ends |
| StartingRadius | 185000 |
| Radius | 0:150000, 1:80000, 2:40000, 3:20000, 4:10000, 5:5000, 6:2500, 7:1650, 8:1090, 9:0 |
| WaitTime (s) | 1:200, 2:120, 3:90, 4:80, 5:50, 6:30, 7+:0 |
| ShrinkTime (s) | 1:240, 2:120, 3:90, 4:70, 5:60, 6:60, 7:55, 8:45, 9:75 |
| Damage (fraction of max health per tick) | 0–2: 0.01, 3: 0.02, 4: 0.05, 5: 0.08, 6–9: 0.1 |
| SupplyDropMinCount / MaxCount | 0: 2 / phases 0–4: 2, 5–6: 1, 7+: 0 |
| Volume.00/01/02.RejectionChance | 0.9 / 0.7 / 0.5 (phase 0 only): chance to reject a random centre that falls in a "bad" volume |
| RejectRadius, ForceDistanceMin/Max, MegaStormGridThickness, StormCamping* | 0 (features off) |
| PlayerCap.Enabled | 0 (off) |

How the server uses it (matches `StartSafeZonesPhase`, D2 card §8): for phase
`n`, hold time = WaitTime(n), shrink time = ShrinkTime(n), next radius =
Radius(n). The next centre is a random point such that the next circle lies
inside the current one, re-rolled when it falls inside a blacklist volume
(`AthenaSafeZoneBlacklist`) with the rejection chance above. The centre
picking function is still to be located in the binary (search the name index
for `SafeZone`); if it turns out to be missing, the rule in this paragraph is
the re-creation and is well known behaviour.

Phases 7–9 have WaitTime 0: these are the moving end-game zones.

## 3. Bus, supply drops, respawn, DBNO (same table)

| Row | Value |
|---|---|
| Aircraft.Height | 80000 |
| Aircraft.Speed | 7500 |
| Aircraft.DeviationAngle | 45 |
| Aircraft.DistanceFromMidLine | 0 |
| SupplyDrop.TimeInterval / TimeDeviation | 180 s / 30 s |
| SupplyDrop.Min/MaxSpawnHeight | 28000 / 32000 |
| SupplyDrop.Min/MaxSpeed | 475 / 525 |
| DBNO.InitialHealth / ReviveHealth | 100 / 30 |
| DBNO.BleedTick / BleedDamage | 1.2 s / 2 |
| RespawnHeight / RespawnTime | 15000 / 5 s |
| TurboBuildInterval / FirstInterval | 0.005 / 0.005 |
| ShieldSmallAmount / ShieldSmallCap | 25 / 0.5 |
| AmmoDespawnTime / AmmoStormDespawnTime | 30 / 15 |

The table has 1,899 rows in total; most are per-item tuning read by item
Blueprints and abilities through `FScalableFloat` handles. The server gets them
for free once the curve table is loaded and registered the same way as on the
client (`ApplyPlaylistDataTables` 0x141934980).

## 4. Other balance tables (`/Game/Athena/Balance/DataTables/`)

| Table | Content |
|---|---|
| AthenaResourceRates | harvest amounts per hit by resource class (ResourceLow 10, Small 11, Medium 19, High 33, Super 72; metal variants), x=1 value 0.15 |
| AthenaFallingDamage | row `Player`: fall speed → damage fraction (1300:0.1, 1536:0.2, 1920:0.5, 2304:1.0) |
| AthenaAccoladeXP | 121 rows of XP per accolade (Default_Basic 80, Default_Medal 210, Reboot 235, …) |
| AthenaAttributesBuildingSection, AthenaHeroAttributeData, AthenaGadgetAttributeData, AthenaVehicleData, AthenaXPPunchCard, AthenaPlaylistXpToggle, AthenaQuestPropAttributeData | not read yet; same method |

## 5. Loot tables

Both tables are complete in the client content (the `_Client` suffix does not
mean stripped: weights and item paths are filled in).

- `AthenaLootTierData_Client`: 175 rows, 54 tier groups. Row struct
  `FortLootTierData`: TierGroup, Weight, LootTier, LootPackage,
  NumLootPackageDrops, LootPackageCategoryWeightArray / MinArray / MaxArray,
  bAllowBonusLootDrops. 81 rows have Weight > 0.
- `AthenaLootPackages_Client`: 1,361 rows, 236 package ids. Row struct
  `FortLootPackageData`: LootPackageID, Weight, Count, LootPackageCategory,
  LootPackageCall (name of another package to roll), ItemDefinition (soft path),
  bAllowBonusDrops. 530 rows have Weight > 0.

Tier groups that matter first: `Loot_AthenaFloorLoot`,
`Loot_AthenaFloorLoot_Warmup`, `Loot_AthenaTreasure` (chest),
`Loot_AthenaAmmoLarge` / `AmmoSmall` (ammo box), `Loot_AthenaSupplyDrop`,
`Loot_AthenaLlama`, `Loot_AthenaVending`, `Loot_ApolloTreasure_Rare`,
`Loot_AthenaIceBox`, `Loot_AthenaMedBox`.

Rolling algorithm (standard Fortnite scheme; check against
`ABuildingContainer::SpawnLoot` 0x141bec150 and the `PickLootDrops` function it
calls when porting):

1. Take all tier rows whose TierGroup equals the container's
   `SearchLootTierGroup` (after the playlist's tier-group redirect map) and
   whose Weight > 0. Pick one by weight.
2. The row gives a LootPackage id and NumLootPackageDrops. The category arrays
   say how many drops must come from each LootPackageCategory (Min array) —
   example chest row: category 0 (weapon) min 1, category 2 and 3 min 1 each.
3. For each drop: among package rows with that LootPackageID and the wanted
   category, pick by weight. If the row has a LootPackageCall, roll that
   package (a `WorldList.*` list) by weight instead. The final row gives
   ItemDefinition and Count.
4. Spawn a pickup for each result (`AFortPickupAthena`), with the weapon's
   loaded ammo from the weapon stats table.

Example read from the data: chest row `Loot_AthenaTreasure_01` (weight 0.3) →
package `WorldPKG.AthenaLoot.Weapon.HighShotgun`, 3 drops; that package calls
`WorldList.AthenaLoot.Weapon.HighShotgun` (items with weights 0.4 / 0.094 / 0)
and an ammo list.

## 6. Work items from this card

1. Server: load and register the playlist's GameData curve table and the two
   composite loot tables at session creation (port ApplyPlaylistDataTables).
   Test: log `Default.SafeZone.Radius` at phase 1 = 80000 on the server.
2. Add a unit-style check script (Python, offline) that rolls every tier group
   10,000 times from the exported tables and prints the item distribution.
   It is the reference for testing the C++ port of the loot roll.
3. Read the remaining balance tables listed in §4 when their system card is
   written.
4. Locate in the binary: safe-zone centre picking, `PickLootDrops`, supply drop
   spawn timing. Names to search in `server_scope/string_names.json`:
   `SafeZone`, `PickLootDrops`, `SupplyDrop`.
