// Original address: 0x1419d53d0
// CONFIDENCE: high
float Result = Super::GetMaxSpeed();

if (MovementMode == MOVE_Walking
	&& JumpPenalties.IsValidIndex(NativeJumpPenaltyIndex1e54)
	&& NativeJumpPenaltyRemaining1e58 > 0.0f)
{
	Result *= JumpPenalties[NativeJumpPenaltyIndex1e54].MovementScalar;
}

if (NativeMeleeSlowRemaining1e5c > 0.0f)
{
	if (const AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
	{
		Result *= AthenaPawn->MeleeCombatSlowSpeedMultiplier;
	}
}

if (MovementMode == MOVE_Flying)
{
	Result *= MaxAccelerationFlyingMultiplier;
}

return Result;
