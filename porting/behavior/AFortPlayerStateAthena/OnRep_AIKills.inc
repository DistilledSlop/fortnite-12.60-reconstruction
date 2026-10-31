// Behaviour snippet: AFortPlayerStateAthena::OnRep_AIKills (kind helper, protected: a non-reflected member; body only).
// Original address: 0x142a8d970 (exec record; its tail jump reaches the body at 0x141a56ed0, read from the runtime dump).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_AIKills.c (body bytes: runtime dump at 0x141a56ed0)
// Proxy score: none (tenth pass; not a portability record)
// Inferred: this function is not a RepNotify (AIKillCount has no ReplicatedUsing), so the generated header declares no
//   UFUNCTION for it and the generator cannot splice it as onrep. It is therefore a helper (protected, flags 0x80401 include
//   FUNC_Protected). Its four steps, in the asm order:
//   1. The owner (AActor::Owner, +0xe0, through GetOwner()) when it IsA AFortPlayerControllerZone: the no-argument native
//      delegate at zone +0x2908 (accessor 0x1423def70) is broadcast with the global Broadcast 0x140684de0.
//   2. AIKillCountNative (+0x11b0) is broadcast with (this, AIKillCount).
//   3. When the playlist of GetCurrentPlaylistAthena() is FirstToGoalScore (WinConditionType 3), the cache AIKillCountPoints
//      (+0xeb4) becomes UpdateScoreForEvent of the AIKilled event (11). Here the PreviousPoints argument is the cache, as
//      in the stat bodies (only OnRep_Kills passes the count).
//   4. UpdateTotalPlayerScore() (tail call 0x141a3c390).
//   Names: AIKillCountPoints and AIKillCountNative are medium (ninth pass). ZoneAIKillCountChangedNative is LOW (our name, not from the binary).
// Declared in generated header: no (helper: the generator declares it under the protected label)
// CONFIDENCE: MEDIUM
AFortPlayerControllerZone* ZoneController = Cast<AFortPlayerControllerZone>(GetOwner());
if (ZoneController != nullptr)
{
	ZoneController->ZoneAIKillCountChangedNative.Broadcast();
}
AIKillCountNative.Broadcast(this, AIKillCount);
AFortGameStateAthena* GameState = GetFortGameStateAthena(this);
if (GameState != nullptr)
{
	UFortPlaylistAthena* Playlist = GameState->GetCurrentPlaylistAthena();
	if (Playlist != nullptr && Playlist->WinConditionType == EAthenaWinCondition::FirstToGoalScore)
	{
		AIKillCountPoints = UpdateScoreForEvent(EAthenaScoringEvent::AIKilled, AIKillCount, AIKillCountPoints, Playlist);
	}
}
UpdateTotalPlayerScore();
