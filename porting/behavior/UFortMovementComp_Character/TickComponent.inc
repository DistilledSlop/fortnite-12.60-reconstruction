// Original address: 0x1420f0970
// CONFIDENCE: high
Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

if (!HasValidData())
{
	return;
}

AFortPawn* FortPawn = Cast<AFortPawn>(CharacterOwner);
if (!FortPawn)
{
	return;
}

const FVector Loc = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector(FLT_MAX);
if (!IsFalling())
{
	if (FortPawn->GetLocalRole() != ROLE_SimulatedProxy
		&& MovementMode == MOVE_Custom && (CustomMovementMode == 1 || CustomMovementMode == 2))
	{
		int32 NewRelation = -1;
		if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(FortPawn))
		{
			const int32 NumBalloons = Player->GetNumActiveBalloons();
			if (NumBalloons > 0)
			{
				const float CeilingZ = GetAirControlParams().CustomGravityCeiling.GetValueAtLevel(static_cast<float>(NumBalloons));
				if (Loc.Z > CeilingZ)
				{
					NewRelation = 0;
				}
			}
		}
		NativeSetGravityCeilingRelation1420edc00(NewRelation);
	}
	FallingStartedZ = -3.4e38f;
}
else
{
	if (Loc.Z > FallingStartedZ)
	{
		FallingStartedZ = Loc.Z;
	}
	if (FortPawn->GetLocalRole() != ROLE_SimulatedProxy && MovementMode == MOVE_Custom && CustomMovementMode == 9)
	{
		int32 NewRelation = -1;
		float MinZ = 0.0f, CenterZ = 0.0f, MaxZ = 0.0f;
		if (NativeGetVortexCeilingRange1420ddab0(GetAirControlParams(), GetAirControlLevel(), MinZ, CenterZ, MaxZ))
		{
			const float RoundedZ = static_cast<float>(FMath::RoundToInt(Loc.Z * 0.2f)) * 5.0f;
			if (RoundedZ >= MinZ)
			{
				NewRelation = (MaxZ > RoundedZ) ? 1 : 0;
			}
		}
		NativeSetGravityCeilingRelation1420edc00(NewRelation);
	}
	if (FortPawn->GetLocalRole() == ROLE_Authority && (NativePackedAccelFlags_1e38 & 1) == 0 && (FallingStartedZ - Loc.Z) > 384.0f)
	{
		NativePackedAccelFlags_1e38 |= 1;
		bTriggeredFallingFeedbackSinceLanded = true;
	}
}
