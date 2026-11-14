// Original address: 0x1420e6e30
// CONFIDENCE: high
if (!HasValidData())
{
	return;
}

NativePackedAccelFlags_1e38 &= ~0x20;

const EMovementMode NewMode = MovementMode;
const uint8 NewCustom = CustomMovementMode;
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);

if (PreviousMovementMode == MOVE_Custom)
{
	switch (PreviousCustomMode)
	{
	case 1:
	case 2:
		NativeClearCustomParachute1420e6d30();
		break;
	case 3:
	case 6:
	case 12:
		if (Player)
		{
			MaxAcceleration = GetDefault<UFortMovementComp_Character>()->MaxAcceleration;
		}
		if (PreviousCustomMode == 3 && UpdatedComponent)
		{
			bUseControllerDesiredRotation = bOrientRotationToMovement;
		}
		break;
	case 4:
	case 5:
		NativePackedAccelFlags_1e38 |= 0x20;
		if (NewMode != MOVE_Custom || (NewCustom != 4 && NewCustom != 5))
		{
			NativeResetCapsuleLocation1420ecae0();
		}
		break;
	case 8:
		NativeSetGravityCeilingRelation1420edc00(0);
		if (Player)
		{
			Player->NativeEndZiplining142314600(false);
		}
		break;
	case 9:
		if (GravityCeilingRelation != -1)
		{
			GravityCeilingRelation = -1;
		}
		break;
	case 10:
		NativeWaterSplineUpdate1420f6bb0();
		if (UpdatedComponent)
		{
			FallingStartedZ = UpdatedComponent->GetComponentLocation().Z;
		}
		if (Player)
		{
			if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				const FGameplayTag& SwimTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x15e8);
				if (ASC->HasMatchingGameplayTag(SwimTag))
				{
					ASC->RemoveLooseGameplayTag(SwimTag);
				}
			}
		}
		break;
	default:
		break;
	}
}

if (NewMode == MOVE_Custom)
{
	switch (NewCustom)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 9:
	case 12:
		CustomFallingAcceleration = Acceleration;
		NativeAirborneVelocity_1e0c = Velocity;
		break;
	case 10:
		NativeEnterSurfaceSwimming1420e66c0();
		break;
	default:
		break;
	}
}

Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

if (IsFalling())
{
	if (UpdatedComponent)
	{
		const float CurrentZ = UpdatedComponent->GetComponentLocation().Z;
		if (CurrentZ > FallingStartedZ)
		{
			FallingStartedZ = CurrentZ;
		}
	}
}

if (PreviousMovementMode == MOVE_Falling && NewMode != MOVE_Falling && NewMode == MOVE_Custom && NewCustom == 10 && Player)
{
	Player->StopJumping();
}

if ((PreviousMovementMode == MOVE_Falling || (NewMode == MOVE_Custom && NewCustom == 10)) && PreviousMovementMode != NewMode && Player)
{
	Player->bIsWaterJump = false;
	Player->NativeEndWaterSprintBoost142314570();
}
