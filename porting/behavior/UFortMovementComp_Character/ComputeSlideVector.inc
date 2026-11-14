// Original address: 0x1420da1d0
// CONFIDENCE: high
FVector SlideResult = Super::ComputeSlideVector(Delta, Time, Normal, Hit);

if (IsMovingOnGround())
{
	if (Cast<AFortPawn>(Hit.Actor.Get()))
	{
		const float SlideSq = SlideResult.SizeSquared();
		if (SlideSq > 0.0001f)
		{
			const float DesiredSpeed = Delta.Size() * 0.75f;
			if (SlideSq < DesiredSpeed * DesiredSpeed)
			{
				return SlideResult.GetSafeNormal() * DesiredSpeed;
			}
		}
	}
	return SlideResult;
}

if (MovementMode == MOVE_Custom && CustomMovementMode == 10 && SlideResult.Z < 0.0f)
{
	const float AbsSlideZ = FMath::Abs(SlideResult.Z);
	const float MaxDepth = SurfaceSwimmingWaterControlParams.MaxImmersionDepth.GetValueAtLevel(0.0f);
	const float CurDepth = NativeRecoveredFloat_0x1d08;
	FVector ClampedDelta = SlideResult;

	if (CurDepth < MaxDepth)
	{
		if (AbsSlideZ + CurDepth > MaxDepth && AbsSlideZ > MaxDepth - CurDepth && AbsSlideZ > 0.0001f)
		{
			const float Scale = (MaxDepth - CurDepth) / AbsSlideZ;
			ClampedDelta *= Scale;
		}
		else
		{
			return SlideResult;
		}
	}
	else
	{
		ClampedDelta = FVector::ZeroVector;
	}

	const FVector Remainder = SlideResult - ClampedDelta;
	const FVector HorizNormal = FVector(Normal.X, Normal.Y, 0.0f).GetSafeNormal();
	const FVector ExtraSlide = Super::ComputeSlideVector(Remainder, 1.0f, HorizNormal, Hit);
	return ClampedDelta + ExtraSlide;
}

return SlideResult;
