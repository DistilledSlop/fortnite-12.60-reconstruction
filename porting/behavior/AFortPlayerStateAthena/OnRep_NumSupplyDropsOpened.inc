// Behaviour snippet: AFortPlayerStateAthena::OnRep_NumSupplyDropsOpened (body only; spliced into the generated member function).
// Original address: 0x142a8dbd0 (exec record of the RepNotify; the same body is the native RepNotify).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_NumSupplyDropsOpened.c
// Proxy score: none (ninth pass; not a portability record)
// Inferred: P_FINISH is the script VM epilogue of an exec thunk and is not part of a native RepNotify, so it is not
//   ported. The supply drop stat (event 4, SupplyDropOpened). The game state comes from GetFortGameStateAthena; its
//   playlist is GetCurrentPlaylistAthena. When that playlist's WinConditionType is FirstToGoalScore (3), the points cache
//   NumSupplyDropsOpenedPoints (+0xecc; the decompile's NumSupplyDropsOpened + 4) becomes UpdateScoreForEvent of the
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
		NumSupplyDropsOpenedPoints = UpdateScoreForEvent(EAthenaScoringEvent::SupplyDropOpened, NumSupplyDropsOpened, NumSupplyDropsOpenedPoints, Playlist);
	}
}
UpdateTotalPlayerScore();
