# minigame_large96_runtime (source drafts, uncounted, build pending, nothing built/tested)

Manifest: `MANIFEST.empty` (empty). No body is closed; none is counted as implementation.
Shared bindings: `Minigame96Bindings.inc` (includes minigame_large94 `AddMinigamePlayer.bindings.inc` unchanged).

| File | Native | Name (from log strings) |
|---|---|---|
| AssignPlayerToAvailableBucket.recovered.cpp | 0x142083810 | AFortMinigame::AssignPlayerToAvailableBucket (inlines GetBestAvailableTeamBucket) |
| DelayedTeleportPlayer.recovered.cpp | 0x142089b10 | AFortMinigame::DelayedTeleportPlayer(TWeakObjectPtr<AFortPlayerState>) |
| SetTeamForPlayer.recovered.cpp | 0x1420a5820 | AFortMinigame::SetTeamForPlayer(PS, NewTeam, bTemporary, bChangeTeamWithoutRespawn) |
| StartMinigameCountdown.recovered.cpp | 0x1420a7410 | AFortMinigame::StartMinigameCountdown(PS) |

Newly bound to reflection (AFortMinigame): PlayerChangedTeamDelegate +0x308 (4-param delegate: PS, TeamId, OldTeamId, bool), PlayerArray +0x438,
PlayerDeathsStatFilter +0x698, AutoStartDelay +0x798, WarmupDuration +0x7a0, TeamArray +0x7c0 (FMinigameTeam 0x28: TeamIndex +0,
bHasBucketAvailable +0x24), CurrentState +0x870, Volume +0x910, bTeamMinigame +0x91a, NumTeams +0xaa0, MinigameStarter +0xaa8, PlayerBuckets +0xc80
(FMinigamePlayerBucket 0x18: TeamIdAtRoundStart +1, DesiredTeamSizePercent +4, PlayerIds +8). UFortRuntimeOptions +0xa5 = bCreativeManualBroadcasterStart;
AFortVolume +0x2d8 bit0 = bNeverAllowSaving. PlayerArray.Players.Num() = Game+0x548.
Timers: DelayedTeleportPlayer = FTimerDelegate::CreateUObject(this,&DelayedTeleportPlayer,Weak), SetTimer(Record+0x88, 0.1f, no loop).

Key fidelity notes: bucket "random" scan keeps Index=first bucket when nothing beats it (never stays -1 with buckets); fallback branch reuses the
register so Index becomes the OLD bucket count (the "Failed to find a bucket" spectate branch is statically dead); IDIVs have no zero guards; SetTeamForPlayer
appends a NULL FUniqueNetIdRepl to matching buckets and removes the player id from the others; TeamIdAtRoundStart is compared (not the index);
playlist-team validation only runs when 0x141641350(world) != 0.

Unresolved (declared in the .inc): 0x1420a6bc0, 0x142700740/0x1429da540/Game+0x228 sub-object, 0x1420830a0 AddBucket, 0x14209b5a0, 0x1420a9a30,
0x142087770, 0x14208d7e0, 0x141c74820/0x141c94c20, 0x141a5b590/0x141a4b2b0, Player+0xd30 byte (unreflected), 0x141641350/0x141996680/0x141641300,
GM vtbl+0xb28, 0x142414c60, 0x1420a4190, 0x141e89b00, 0x142099810, 0x1429bb7f0, globals _DAT_1478c5044/_DAT_1478c5014, Volume +0x408, Game +0x8b0/+0xb38,
countdown lambda body (vtbl 0x14608f968/9c0), log category 0x14813df34 & varargs (decompiler dropped Logf args; inferred), 12.60 SetTeamForPlayer param roles
(inferred from native usage; 23.20 SDK has 5 params). Not yet checked: that these four are declared UFUNCTIONs/members in 12.60 AFortMinigame.h.

Root review removed the inferred countdown callback implementation. It now calls an explicitly unresolved factory, retaining the proven weak capture shape without guessing the callback body. These four source drafts remain uncounted. CLI Sonnet medium finished successfully; root assigned a fresh bounded player/death/streaming batch next.
