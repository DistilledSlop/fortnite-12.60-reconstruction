// CONFIDENCE: LOW
// Original target 0x1419f13d0.
if (DBNOPawn == nullptr)
{
	return;
}
// Decrement the DBNO revive counter (byte at the pawn's DBNO component); when it reaches zero, finish the revive (FUN_141a0e630).
if (DBNOPawn->ReviveCounter_0x4 != 0)
{
	DBNOPawn->ReviveCounter_0x4 -= 1;
	if (DBNOPawn->ReviveCounter_0x4 == 0)
	{
		Helper_141a0e630(DBNOPawn);
	}
}
// Player state: set revive state to Interrupted (value 4) through FUN_141a3d480.
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
{
	Helper_141a3d480(PlayerState, 4);
}
// Telemetry only when the runtime option at +0x837 is enabled.
if (UFortRuntimeOptions_GetDefault()->bLogRevives_0x837)
{
	if (auto* Mode = GetWorld()->Mode_0x188)
	{
		if (auto* Reviver = DBNOPawn->Reviver_0xe0)
		{
			if (PlayerState != nullptr)
			{
				// Format: "PlayerRevived,<reviver name>,<reviver id>,<victim name>,<victim id>,false".
				FString Message = FormatString(TEXT("PlayerRevived,%s,%d,%s,%d,%s"),
					Reviver->PlayerName, Reviver->PlayerId_0x336, PlayerState->PlayerName, PlayerState->PlayerId_0x336, TEXT("false"));
				Helper_141e4f8a0(Mode, Message);
			}
		}
	}
}
