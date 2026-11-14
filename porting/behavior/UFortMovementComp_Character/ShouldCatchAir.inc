// Original address: 0x1420eee30
// CONFIDENCE: high
if (OldFloor.IsWalkableFloor())
{
	if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
	{
		if (Player->bIsInSlipperyMovement)
		{
			const FVector& N1 = OldFloor.HitResult.ImpactNormal;
			const FVector& N2 = NewFloor.HitResult.ImpactNormal;
			if (!N1.Equals(N2, 0.001f) && N1.Z > 0.0001f && N2.Z > 0.0001f)
			{
				const float HorizDot1 = N1.X * Velocity.X + N1.Y * Velocity.Y;
				const float HorizDot2 = N2.X * Velocity.X + N2.Y * Velocity.Y;
				const float SlopeZ1 = -HorizDot1 / N1.Z;
				const float SlopeZ2 = -HorizDot2 / N2.Z;
				if (SlopeZ2 < SlopeZ1)
				{
					const FVector Dir1 = FVector(Velocity.X, Velocity.Y, SlopeZ1).GetSafeNormal();
					const FVector Dir2 = FVector(Velocity.X, Velocity.Y, SlopeZ2).GetSafeNormal();
					const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Dir1, Dir2), -1.0f, 1.0f)));
					const float MinAngle = Player->SlipperySlopeParams.SlopeLaunchMinRequiredAngleChange.GetValueAtLevel(0.0f);
					const float MinSpeed = Player->SlipperySlopeParams.SlopeLaunchMinRequiredSpeed.GetValueAtLevel(0.0f);
					if (MinAngle >= 0.0f && AngleDeg >= MinAngle && MinSpeed > 0.0f && Velocity.SizeSquared2D() >= MinSpeed * MinSpeed)
					{
						return true;
					}
				}
			}
		}
	}
}
return false;
