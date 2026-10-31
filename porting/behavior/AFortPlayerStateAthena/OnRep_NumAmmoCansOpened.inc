// Behaviour snippet: AFortPlayerStateAthena::OnRep_NumAmmoCansOpened (body only; spliced into the generated member function).
// Original address: 0x142a8dad0 (exec record of the RepNotify; the same body is the native RepNotify).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_NumAmmoCansOpened.c
// Proxy score: none (ninth pass; not a portability record)
// Inferred: P_FINISH is the script VM epilogue of an exec thunk and is not part of a native RepNotify, so it is not
//   ported. The Ammo stat (event 3, AmmoCanOpened). The game state comes from GetFortGameStateAthena; its playlist is
//   GetCurrentPlaylistAthena. When that playlist's WinConditionType is FirstToGoalScore (3), the points cache
//   NumAmmoCansOpenedPoints (+0xec4; the decompile's NumAmmoCansOpened + 4) becomes UpdateScoreForEvent of the stat.
//   UpdateTotalPlayerScore runs in every case. The decompile passes the object to the accessor in this body without
//   showing the argument; the accessor takes the object it is called on, so the object is this (see the Chests body).
//   Field names are medium confidence (see porting/decompilation_notes.md, ninth pass, naming table).
// Declared in generated header: yes
// CONFIDENCE: HIGH
AFortGameStateAthena* GameState = GetFortGameStateAthena(this);
if (GameState != nullptr)
{
	UFortPlaylistAthena* Playlist = GameState->GetCurrentPlaylistAthena();
	if (Playlist != nullptr && Playlist->WinConditionType == EAthenaWinCondition::FirstToGoalScore)
	{
		NumAmmoCansOpenedPoints = UpdateScoreForEvent(EAthenaScoringEvent::AmmoCanOpened, NumAmmoCansOpened, NumAmmoCansOpenedPoints, Playlist);
	}
}
UpdateTotalPlayerScore();
