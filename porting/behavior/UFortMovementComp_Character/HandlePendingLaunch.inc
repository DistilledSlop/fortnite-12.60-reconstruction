// Original address: 0x1420e36c0
// CONFIDENCE: high
if (!PendingLaunchVelocity.IsZero() && HasValidData())
{
	AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
	if (MovementMode == MOVE_Custom && (CustomMovementMode == 1 || CustomMovementMode == 2))
	{
		PendingLaunchVelocity = FVector::ZeroVector;
		if (Player)
		{
			Player->NativeSetPendingSkydiveLaunch14234e8b0(false);
		}
		return false;
	}

	Velocity = PendingLaunchVelocity;
	PendingLaunchVelocity = FVector::ZeroVector;

	if (MovementMode != MOVE_Custom || CustomMovementMode != 12)
	{
		if (Player && Player->bIsInVortex)
		{
			return true;
		}
		if (Player && Player->GetLocalRole() == ROLE_Authority && Player->IsParachuteForcedOpen())
		{
			Player->NativeCloseParachute14230e850();
		}
		SetMovementMode(MOVE_Falling, 0);
	}
	return true;
}
return false;
