// Original address: 0x1420eb9b0
// CONFIDENCE: high
const bool bCustomAirMode = MovementMode == MOVE_Custom && (
	CustomMovementMode == 3 ||
	CustomMovementMode == 4 ||
	CustomMovementMode == 5 ||
	CustomMovementMode == 9 ||
	CustomMovementMode == 12);

if (bCustomAirMode && !NativeCanStepUpOrLandOn1420e57d0(Hit))
{
	return;
}

if (AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
{
	if (AthenaPawn->GetLocalRole() == ROLE_Authority)
	{
		if (ABuildingSMActor* Building = Cast<ABuildingSMActor>(AthenaPawn->NativeGetFloorActor142316940(nullptr)))
		{
			if (Building->NativeFloorAttributionWeak718.IsValid())
			{
				AthenaPawn->FallInstigator = Building->NativeFloorAttributionWeak718.Get();
			}
			AthenaPawn->LastFloorBeforeFalling = Building;
		}
	}
}

NativeWaterSplineUpdate1420f6bb0();
if (NativeTryEnterSurfaceSwimming1420d9940())
{
	return;
}

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
bool bAppliedSlipperyLanding = false;
if (Player && Player->bIsInSlipperyMovement && Hit.ImpactNormal.Z > 0.0001f)
{
	bAppliedSlipperyLanding = true;
	const FVector2D HitN2D(Hit.ImpactNormal.X, Hit.ImpactNormal.Y);
	const float ProjDot = HitN2D.X * Velocity.X + HitN2D.Y * Velocity.Y;
	if (ProjDot < 0.0f)
	{
		Velocity.X -= HitN2D.X * ProjDot;
		Velocity.Y -= HitN2D.Y * ProjDot;
	}
	const float ForceScalar = Player->SlipperySlopeParams.SlopeLandingForceScalar.GetValueAtLevel(0.0f);
	const float MaxForce = Player->SlipperySlopeParams.SlopeLandingMaxHorizontalForce.GetValueAtLevel(0.0f);
	const float DownForce = Velocity.Z * Hit.ImpactNormal.Z;
	const float AddX = FMath::Clamp(-HitN2D.X * DownForce * ForceScalar, -MaxForce, MaxForce);
	const float AddY = FMath::Clamp(-HitN2D.Y * DownForce * ForceScalar, -MaxForce, MaxForce);
	const FVector2D OldHoriz(Velocity.X, Velocity.Y);
	Velocity.X += AddX;
	Velocity.Y += AddY;

	const float MaxSpeed = Player->SlipperySlopeParams.MaxLateralSpeed.GetValueAtLevel(0.0f);
	const float NewHorizSq = Velocity.SizeSquared2D();
	if (NewHorizSq > MaxSpeed * MaxSpeed)
	{
		const float AllowedSpeed = FMath::Max(OldHoriz.Size(), MaxSpeed);
		if (AllowedSpeed >= 0.0001f)
		{
			const FVector2D Clamped = FVector2D(Velocity.X, Velocity.Y).GetSafeNormal() * AllowedSpeed;
			Velocity.X = Clamped.X;
			Velocity.Y = Clamped.Y;
		}
		else
		{
			Velocity.X = 0.0f;
			Velocity.Y = 0.0f;
		}
	}
}

if (!bAppliedSlipperyLanding)
{
	const float MaxSpeed = GetMaxSpeed();
	if (MaxSpeed >= 0.0001f)
	{
		if (Velocity.SizeSquared2D() > MaxSpeed * MaxSpeed)
		{
			const FVector2D Clamped = FVector2D(Velocity.X, Velocity.Y).GetSafeNormal() * MaxSpeed;
			Velocity.X = Clamped.X;
			Velocity.Y = Clamped.Y;
		}
	}
	else
	{
		Velocity.X = 0.0f;
		Velocity.Y = 0.0f;
	}
}

Super::ProcessLanded(Hit, remainingTime, Iterations);
NativePackedAccelFlags_1e38 &= ~0x1;
bTriggeredFallingFeedbackSinceLanded = false;
