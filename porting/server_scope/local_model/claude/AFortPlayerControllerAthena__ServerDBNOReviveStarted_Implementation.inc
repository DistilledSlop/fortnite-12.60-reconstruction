// CONFIDENCE: LOW
// Original target 0x1419f1600.
if (DBNOPawn == nullptr)
{
	return;
}
DBNOPawn->ReviveCounter_0x4 += 1;
if (DBNOPawn->ReviveCounter_0x4 == 1)
{
	// Reviver-side controller from the pawn's accessor (vfunc at +0x1e of the DBNO component).
	if (auto* ReviverController = DBNOPawn->GetReviverController())
	{
		auto* Global = GetThreadLocalGameplayGlobals();	// FUN_141ed1800; field +0x518 is the revive descriptor
		if (Helper_140744d30(ReviverController->ReviveList_0x208, Global->ReviveDescriptor_0x518, 1))
		{
			ReviverController->NativeStartRevive(Global->ReviveDescriptor_0x518, 1);	// vslot 0x810
		}
		// Player state flag: set revive state to Starting (value 4) through FUN_141a4bb60.
		if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
		{
			Helper_141a4bb60(PlayerState, 4);
		}
		// Revive timer: current time from FUN_141ef6bc0(this), plus the pawn's configured revive duration.
		DBNOPawn->ReviveEndTime = Helper_141ef6bc0(this) + DBNOPawn->ReviveDuration;	// float write (offset unresolved)
		// FUN_1406b5460 starts the revive event with the global descriptor at +0x1520.
		Helper_1406b5460(ReviverController, Global->ReviveEventDesc_0x1520, &ReviveParams);
		if (UFortRuntimeOptions_GetDefault()->bLogRevives_0x837)
		{
			if (auto* Mode = GetWorld()->Mode_0x188)
			{
				if (auto* Reviver = DBNOPawn->Reviver_0x70)
				{
					if (PlayerState != nullptr)
					{
						// "PlayerBeingRevived,%s,%d,%s,%d"
						FString Message = FormatString(TEXT("PlayerBeingRevived,%s,%d,%s,%d"),
							Reviver->PlayerName, Reviver->PlayerId_0x336, PlayerState->PlayerName, PlayerState->PlayerId_0x336);
						Helper_141e4f8a0(Mode, Message);
					}
				}
			}
		}
	}
}
// Common tail (runs whenever DBNOPawn is valid): FUN_141a24720 refreshes the pawn's revive presentation.
Helper_141a24720(DBNOPawn);
