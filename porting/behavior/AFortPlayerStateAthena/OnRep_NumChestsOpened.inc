// Behaviour snippet: AFortPlayerStateAthena::OnRep_NumChestsOpened (body only; spliced into the generated member function).
// Original address: 0x142a8db10 (exec record of the RepNotify; the same body is the native RepNotify).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_NumChestsOpened.c
// Proxy score: none (ninth pass; not a portability record)
// Inferred: P_FINISH is the script VM epilogue of an exec thunk and is not part of a native RepNotify, so it is not
//   ported. The chest stat (event 2, ChestOpened). The game state comes from GetFortGameStateAthena; its playlist is
//   GetCurrentPlaylistAthena. When that playlist's WinConditionType is FirstToGoalScore (3), the points cache
//   NumChestsOpenedPoints (+0xebc; the decompile's NumChestsOpened + 4) becomes UpdateScoreForEvent of the stat.
//   UpdateTotalPlayerScore runs in every case.
//   NO-OP (decided in the ninth pass, recommended by the Eighth pass): the decompile wraps the body in a profiling
//   scope (STAT_InvNPerfHook_PSA_RCO, begun and ended by 0x142cd3770 only when the byte at 0x1482472e4 is zero). That
//   wrapper only times the body, so it is not ported. The scope call 0x142cd3770 is not ported at all.
//   Field names are medium confidence (see porting/decompilation_notes.md, ninth pass, naming table).
// Declared in generated header: yes
// CONFIDENCE: HIGH
AFortGameStateAthena* GameState = GetFortGameStateAthena(this);
if (GameState != nullptr)
{
	UFortPlaylistAthena* Playlist = GameState->GetCurrentPlaylistAthena();
	if (Playlist != nullptr && Playlist->WinConditionType == EAthenaWinCondition::FirstToGoalScore)
	{
		NumChestsOpenedPoints = UpdateScoreForEvent(EAthenaScoringEvent::ChestOpened, NumChestsOpened, NumChestsOpenedPoints, Playlist);
	}
}
UpdateTotalPlayerScore();
