# minigame_large94_runtime (source-only, build pending, nothing built/tested)

Manifest: `MANIFEST.empty` (empty) — neither body is closed.

## AFortMinigame::AddOrOverrideEndConditionForTeam (impl 0x142082df0)
`AFortMinigame_AddOrOverrideEndConditionForTeam.inc` is a full-control-flow lift (frame: StatQuery +8, TeamIndex +0).
Array +0x678, element 0x30 = FMinigameEndCondition {StatQuery 0x18, TeamListType +0x18, TeamList +0x20}.
Per element: match = validated Stat class equal && Scope equal. Blacklist(0): query-equal -> empty list, record index A;
else AddUnique(team). Whitelist(1): query-equal -> AddUnique(team), record index B; else empty list. Other type: nothing.
Return only if A and B both set; otherwise append new entry with query copy, TeamListType=1 (Whitelist), TeamList={team}.
(Differs from the older 0x142082c90 `.inc` which appends Blacklist.) Bound: ValidatedStatClass (Minigame94EndConditionBindings.inc).
Unresolved: `0x1420802c0` (query match vs existing condition) and `0x1420a1cb0` (empty/reset TeamList).
Array growth helpers 0x14072b840 / 0x14069e940 are plain TArray::Add growth.

## AddMinigamePlayer (impl 0x142081fa0)
`AddMinigamePlayer.bindings.inc` + `AddMinigamePlayer.recovered.cpp`. Newly bound from reflection/engine:
PlayerArray/Spectating arrays, CurrentState enum names, Volume +0x910, PlayerBuckets +0xc80 (element 0x18; old "team array"),
OnPlayerJoinInProgress +0x348 (the "append pawn" is Broadcast(APawn*)), OnTeamIndexChangedDelegate (Athena PS +0xd90) and
OnBeginSpectatingEvent (Zone PS +0xb48) AddDynamic to reflected UFUNCTIONs OnPlayerStateTeamIndexChanged / OnBeginSpectating,
FMinigamePlayer fields (+0x38 bHasValidResetData, +0x39 ClassSlotIndex, +0x40 TeamBeforeMinigameStarted, +0x44/+0x50 location/rotation,
+0x5c/+0x5f bools, +0x70 SolPlayer, with static_asserts), PartyOwnerUniqueId (PS +0x338, used by policy gate), authority, Athena casts,
NSLOCTEXT join-too-late text, MarkItemDirty, ClientMinigameStartedText copy/release (+0xb88).
Corrections to the old proposal: +0xaa5 is PlayerPersistence; assignment team = PlayerBuckets[idx].TeamIdAtRoundStart;
+0x39 is ClassSlotIndex; Mode(+0xb10 int32)/PreferredTeam(+0xb14) have no reflected property (raw reads only, meaning unproven).
Caveats: bool offsets 0x38/0x5c/0x5f inferred from packing; GetActorLocation/Rotation fallbacks `_DAT_148218630/640` unread;
GetUniqueId() for Player+0x248 assumed (4.26 APlayerState); record +0x78/+0x80 unreflected.

### Remaining unresolved helper addresses (declared, not implemented)
AddMinigamePlayer: 0x1420898c0 FindOrCreateRecord, 0x1420935b0 IsRegistered, vtbl+0x150 WorldContext, 0x1418563c0 PlayerPawn,
0x142419ef0 MakeIdentity, 0x1420997a0 RejectLateJoin (+ global 0x1478c5050), ctrl vtbl+0xa58 message, 0x14297b320 + Volume vtbl+0x610 (+0x23a),
0x142ec0440, 0x142b8c150/0x142ed7360 (SolPlayer), slot helper (0x1428ea7f0, 0x140e2d090, 0x141c8cc90, 0x141ffa200, 0x140d7c5a0 identity eq),
Player+0x318 vtbl+0x28, 0x141a3d510, Pawn vtbl+0xa80 (+0x141ed1800), 0x142a6e250, 0x142083810, 0x1420a5820, 0x142083e10,
0x142089b10, Controller +0x2e78, scheduling (delegate vtbls 0x1459cb3b8/0x1459cb410, 0x142bad740, 0x144627d90),
ctrl vtbl+0x1be0, 0x1420a7410, log category 0x14813df34 + string helpers 0x142ce9f90/0x1444ac7a0.
EndCondition: 0x1420802c0, 0x1420a1cb0 (0x1429d45e0 assumed UFortMinigameStatFilter::StaticClass, body not decompiled here).

Root review: native localized join text confirms namespace/key/source exactly; team-list branch and append cases checked against native142082df0. EMinigamePlayerPersistence is None=0, PartyLeaderOnly=1, AllPlayers=2. Bool packing, native-record tail, assumed class getter and inferred engine bindings remain provisional. No layout assertions have been compiled. The cached text touch models copy/release but requires final ownership review. This is source recovery progress, not an integrated player-registration implementation. Claude runner finished exit0,511.56s, model claude-sonnet-5-5 (CLI sonnet alias), no tests/builds.
