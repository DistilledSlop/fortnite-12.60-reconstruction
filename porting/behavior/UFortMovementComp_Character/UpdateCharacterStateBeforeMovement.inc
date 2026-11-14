// Original address: 0x1420f48b0
// CONFIDENCE: high
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
const bool bIsSimProxy = (Player && Player->GetLocalRole() == ROLE_SimulatedProxy);

Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

const bool bCustomAirMode = MovementMode == MOVE_Custom && (
	CustomMovementMode == 3 ||
	CustomMovementMode == 4 ||
	CustomMovementMode == 5 ||
	CustomMovementMode == 9 ||
	CustomMovementMode == 12);
if (bCustomAirMode)
{
	const FAirControlParams& AirParams = GetAirControlParams();
	MaxAcceleration = AirParams.MaxAcceleration.GetValueAtLevel(GetAirControlLevel());
}

NativeWaterSplineUpdate1420f6bb0();

if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
{
	if (!bIsSimProxy && Player)
	{
		Player->NativeTickWaterSprintBoostTime14235bf50(DeltaSeconds);
	}
	if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
	{
		bool bKeepSwimming = false;
		if (NativeHasSurfaceWater1420e5490())
		{
			if (NativeRecoveredFloat_0x1d08 >= 0.0f && Velocity.Z > 0.0f)
			{
				bKeepSwimming = true;
			}
			else if (Player)
			{
				if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
				{
					const FGameplayTag& SwimTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x15e8);
					if (ASC->HasMatchingGameplayTag(SwimTag))
					{
						bKeepSwimming = true;
					}
				}
			}
		}
		if (!bKeepSwimming)
		{
			if (Player)
			{
				Player->StopJumping();
			}
			SetMovementMode(MOVE_Falling, 0);
			if (UpdatedComponent)
			{
				FallingStartedZ = UpdatedComponent->GetComponentLocation().Z;
			}
		}
	}
}
else
{
	NativeTryEnterSurfaceSwimming1420d9940();
}
