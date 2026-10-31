// Behaviour snippet: AFortPlayerStateAthena::OnRep_Kills (body only; spliced into the generated member function).
// Original address: 0x142a8da10 (exec record of the RepNotify; its tail jump reaches the body at 0x141a51910).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_Kills.c (body bytes: runtime dump at 0x141a51910, read with objdump)
// Proxy score: none (tenth pass; not a portability record)
// Inferred: P_FINISH is the script epilogue of the exec thunk and is not part of the native RepNotify, so it is not ported.
//   The asm (0x141a51910) runs four steps in this order:
//   1. GetFortGameStateAthena(this) (rcx still holds this at the call). When the playlist of GetCurrentPlaylistAthena()
//      has WinConditionType 3 (FirstToGoalScore), the points cache KillScorePoints (+0xe88) becomes UpdateScoreForEvent
//      of the Elimination event (1). The asm loads KillScore (+0xe74) into BOTH the Count and the PreviousPoints argument
//      (r8d and r9d), so the port passes KillScore twice. That is the shipping code; it is kept as it is (quirk).
//   2. UpdateTotalPlayerScore().
//   3. The owner (AActor::Owner at +0xe0, read through GetOwner()) when it IsA AFortPlayerControllerZone: the no-argument
//      native delegate of the zone controller at +0x28f0 (accessor 0x1423def80, `this + 0x28f0`) is broadcast with the
//      global Broadcast 0x140684de0 (TMulticastDelegate<void>::Broadcast). The Cast below is that IsA test plus the cast.
//   4. KillScoreNative (+0x1120) is broadcast with (this, KillScore): the tail call of 0x1407ac640.
//   Names: KillScorePoints and KillScoreNative are medium (ninth pass). ZoneKillScoreChangedNative is LOW (our name, not from the binary).
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
AFortGameStateAthena* GameState = GetFortGameStateAthena(this);
if (GameState != nullptr)
{
	UFortPlaylistAthena* Playlist = GameState->GetCurrentPlaylistAthena();
	if (Playlist != nullptr && Playlist->WinConditionType == EAthenaWinCondition::FirstToGoalScore)
	{
		KillScorePoints = UpdateScoreForEvent(EAthenaScoringEvent::Elimination, KillScore, KillScore, Playlist);
	}
}
UpdateTotalPlayerScore();
AFortPlayerControllerZone* ZoneController = Cast<AFortPlayerControllerZone>(GetOwner());
if (ZoneController != nullptr)
{
	ZoneController->ZoneKillScoreChangedNative.Broadcast();
}
KillScoreNative.Broadcast(this, KillScore);
