// Original address: 0x1419dfce0
// CONFIDENCE: high
bool bEffectiveSweep = bSweep;
if (bEffectiveSweep)
{
	if (const AFortPawn* FortPawn = Cast<AFortPawn>(CharacterOwner))
	{
		if (FortPawn->GetLocalRole() == ROLE_SimulatedProxy)
		{
			if ((NativeExtraFlags_1e39 & 2) == 0)
			{
				bEffectiveSweep = false;
			}
			else if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(FortPawn))
			{
				if (Player->bIsSkydivingFromBus && Player->bIsSkydiving && !Player->bIsParachuteOpen)
				{
					bEffectiveSweep = false;
				}
			}
		}
	}
}
return Super::MoveUpdatedComponentImpl(Delta, NewRotation, bEffectiveSweep, OutHit, Teleport);
