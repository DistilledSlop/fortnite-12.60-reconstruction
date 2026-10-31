// Behaviour snippet: AFortPlayerStateAthena::OnRep_NumGoldCoinsCollected (body only; spliced into the generated member function).
// Original address: 0x142a8db50 (exec record of the RepNotify; the same body is the native RepNotify).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_NumGoldCoinsCollected.c
// Proxy score: none (ninth pass; not a portability record)
// Inferred: P_FINISH is the script VM epilogue of an exec thunk and is not part of a native RepNotify, so it is not
//   ported. The gold coin stat (event 10, CollectedCoinGold). The game state comes from GetFortGameStateAthena; its
//   playlist is GetCurrentPlaylistAthena. When that playlist's WinConditionType is FirstToGoalScore (3), the points cache
//   NumGoldCoinsCollectedPoints (+0xefc; the decompile's NumGoldCoinsCollected + 4) becomes UpdateScoreForEvent of the
//   stat. UpdateTotalPlayerScore runs in every case.
//   Field names are medium confidence (see porting/decompilation_notes.md, ninth pass, naming table).
// Declared in generated header: yes
// CONFIDENCE: HIGH
AFortGameStateAthena* GameState = GetFortGameStateAthena(this);
if (GameState != nullptr)
{
	UFortPlaylistAthena* Playlist = GameState->GetCurrentPlaylistAthena();
	if (Playlist != nullptr && Playlist->WinConditionType == EAthenaWinCondition::FirstToGoalScore)
	{
		NumGoldCoinsCollectedPoints = UpdateScoreForEvent(EAthenaScoringEvent::CollectedCoinGold, NumGoldCoinsCollected, NumGoldCoinsCollectedPoints, Playlist);
	}
}
UpdateTotalPlayerScore();
