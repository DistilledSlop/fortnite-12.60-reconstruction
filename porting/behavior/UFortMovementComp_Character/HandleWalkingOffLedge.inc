// Original address: 0x1420e3b30
// CONFIDENCE: high
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
	if (Player->bIsInSlipperyMovement)
	{
		if (PreviousFloorImpactNormal.Z > 0.0001f)
		{
			Velocity = NativeComputeSlipperyJump1420da5d0(Velocity, PreviousFloorImpactNormal, *Player);
			const float MoveZ = (Velocity.Z <= 0.1f) ? 1.0f : 8.0f;
			if (UpdatedComponent)
			{
				FHitResult Hit;
				SafeMoveUpdatedComponent(FVector(0.0f, 0.0f, MoveZ), UpdatedComponent->GetComponentQuat(), true, Hit);
			}
		}
	}
}
Super::HandleWalkingOffLedge(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
