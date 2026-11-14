// Original address: 0x1419ec080
// CONFIDENCE: high
if (const AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
{
	if (AthenaPawn->IsBotControlled())
	{
		SetGroundMovementMode(MOVE_NavWalking);
	}
}
Super::ProcessLanded(Hit, remainingTime, Iterations);
