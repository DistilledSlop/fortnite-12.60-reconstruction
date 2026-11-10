// dep AFortGameStateZone__OnWaveEncounterStarted_Implementation original 0x141ea72e0
// CONFIDENCE: MEDIUM. Original 0x141ea72e0 (vtable +0xa00). WaveData is not forwarded to the Blueprint event.
if (Role != ROLE_Authority)
{
	return;
}
FText LevelTextCopy = LevelText;
FText WaveTextCopy = WaveText;
FOnWaveEncounterStartedParams Params;
Params.LevelText = LevelTextCopy;
Params.Level = Level;
Params.WaveText = WaveTextCopy;
Params.WaveNum = WaveNum;
ProcessEvent(FindFunctionChecked(OnWaveEncounterStartedFunctionName), &Params);
