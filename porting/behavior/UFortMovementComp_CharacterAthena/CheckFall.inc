// Original address: 0x1419c40d0
// CONFIDENCE: high
const bool bResult = Super::CheckFall(OldFloor, Hit, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump);
if (bResult)
{
	if (ABuildingSMActor* Building = Cast<ABuildingSMActor>(Hit.Actor.Get()))
	{
		if (AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
		{
			if (Building->NativeFloorAttributionWeak718.IsValid())
			{
				AthenaPawn->FallInstigator = Building->NativeFloorAttributionWeak718.Get();
			}
			AthenaPawn->LastFloorBeforeFalling = Building;
		}
	}
}
return bResult;
