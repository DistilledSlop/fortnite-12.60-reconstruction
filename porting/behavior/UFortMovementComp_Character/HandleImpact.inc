// Original address: 0x1420e2d80
// CONFIDENCE: high
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (Player)
{
	const bool bIsWaterJump = Player->bIsWaterJump;
	if (MoveDelta.Z > 0.0f && (IsFalling() || Player->IsJumpProvidingForce() || bIsWaterJump) && Hit.Normal.Z < -0.01f)
	{
		if (Player->bPressedJump)
		{
			Player->StopJumping();
		}
		Player->bIsWaterJump = false;
	}
}

if (Player && Player->GetLocalRole() != ROLE_SimulatedProxy)
{
	if (MovementMode == MOVE_Falling)
	{
		bool bResetStartZ = (NativeWaterSplineKey_1d04 >= 0.0f);
		if (!bResetStartZ && UpdatedPrimitive)
		{
			for (const FOverlapInfo& Overlap : UpdatedPrimitive->GetOverlapInfos())
			{
				if (Overlap.OverlapInfo.GetActor() && Overlap.OverlapInfo.GetActor()->IsA<AFortWaterBodyActor>())
				{
					bResetStartZ = true;
					break;
				}
			}
		}
		if (bResetStartZ)
		{
			FallingStartedZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
		}

		NativeLastImpactTimeSlice_1e18 = TimeSlice;

		bool bIgnoreFallDamage = (Player->bIsDBNO);
		if (!bIgnoreFallDamage)
		{
			if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				const FGameplayTag& IgnoreTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x550);
				if (ASC->HasMatchingGameplayTag(IgnoreTag))
				{
					bIgnoreFallDamage = true;
				}
			}
		}

		if (!bIgnoreFallDamage && !bResetStartZ)
		{
			const float CapsuleZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
			const float FallDist = FMath::Max(FallingStartedZ - CapsuleZ, 0.0f);
			if (FallDist > 0.0f && Hit.ImpactNormal.Z >= GetWalkableFloorZ() && Hit.Time > 0.01f)
			{
				if (Player->GetLocalRole() == ROLE_Authority)
				{
					if (AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(Player))
					{
						if (!AthenaPawn->FallInstigator && AthenaPawn->LastFloorBeforeFalling)
						{
							if (AthenaPawn->LastFloorBeforeFalling->NativeFloorAttributionWeak718.IsValid())
							{
								AthenaPawn->FallInstigator = AthenaPawn->LastFloorBeforeFalling->NativeFloorAttributionWeak718.Get();
							}
						}
					}
				}
				NativeResetFallingStartedZ1420edbc0();
			}
		}

		if (Hit.ImpactNormal.Z < GetWalkableFloorZ()
			&& Velocity.Z <= SlideIfVelocityLessThanZ && Velocity.Z != SlideIfVelocityLessThanZ
			&& Hit.ImpactNormal.Z >= FallingSlopeSafeSlideNormalZ
			&& Hit.Time > 0.01f
			&& !Player->bIsDBNO)
		{
			Player->NativeSetIsSlopeSliding14234dae0(true);
			if (UWorld* World = GetWorld())
			{
				NativeSlideStartTime_1e1c = World->GetTimeSeconds();
			}
			NativeSlideImpactNormal_1e20 = Hit.ImpactNormal;
		}
	}
	else if (MovementMode == MOVE_Custom && CustomMovementMode == 4)
	{
		if (Hit.Normal.Z <= -0.9999f && Player->IsSkydivingFromLaunchPad())
		{
			Player->NativeCloseParachute14230e850();
			SetMovementMode(MOVE_Falling, 0);
		}
	}
	else if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
	{
		if (Hit.Normal.Z > 0.0001f)
		{
			const float CancelDepth = (Player->bIsDBNO
				? SurfaceSwimmingWaterControlParams.CancelCrouchImmersionDepth
				: SurfaceSwimmingWaterControlParams.TargetImmersionDepth).GetValueAtLevel(0.0f);
			if (NativeRecoveredFloat_0x1d08 < CancelDepth)
			{
				const FVector Loc = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
				if (IsValidLandingSpot(Loc, Hit))
				{
					SetMovementMode(MOVE_Walking, 0);
				}
				else
				{
					const float MinDepth = SurfaceSwimmingWaterControlParams.MinImmersionDepth.GetValueAtLevel(0.0f);
					const float CheckDepth = FMath::Clamp(MinDepth, 10.0f, CancelDepth - 1.0f);
					if (NativeRecoveredFloat_0x1d08 < CheckDepth)
					{
						Player->StopJumping();
						SetMovementMode(MOVE_Falling, 0);
					}
				}
			}
		}
	}
}

Super::HandleImpact(Hit, TimeSlice, MoveDelta);
