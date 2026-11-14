// Original address: 0x1420e38d0
// CONFIDENCE: high
FVector Result = Super::HandleSlopeBoosting(SlideResult, Delta, Time, Normal, Hit);

if (MovementMode == MOVE_Falling
	&& Cast<AFortPlayerPawn>(CharacterOwner)
	&& Velocity.Z < 0.0f
	&& NativeLastImpactTimeSlice_1e18 > 0.0001f
	&& Time > 0.0001f)
{
	const float MaxSafeSpeed = FallingSlopeSafeSlideMaxSpeedScalable.GetValueAtLevel(0.0f);
	if (Hit.ImpactNormal.Z >= FallingSlopeSafeSlideNormalZ && Hit.Time > 0.01f && MaxSafeSpeed > 0.0f)
	{
		const float EffectiveTime = Time * NativeLastImpactTimeSlice_1e18;
		FVector DesiredVel = Result / EffectiveTime;
		if (MaxSafeSpeed >= 0.0001f)
		{
			const float SpeedSq = DesiredVel.SizeSquared();
			if (SpeedSq > MaxSafeSpeed * MaxSafeSpeed)
			{
				DesiredVel = DesiredVel.GetSafeNormal() * MaxSafeSpeed;
			}
		}
		else
		{
			DesiredVel = FVector::ZeroVector;
		}
		Result = DesiredVel * EffectiveTime;
	}
}

return Result;
