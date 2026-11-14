// Original address: 0x1420edf10
// CONFIDENCE: high
if (MovementMode == NewMovementMode && (NewMovementMode != MOVE_Custom || CustomMovementMode == NewCustomMode))
{
	return;
}

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (Player && Player->GetLocalRole() != ROLE_SimulatedProxy && NativeModeSwitchGuard_1dfc == 0)
{
	NativeModeSwitchGuard_1dfc = 1;

	if (Player->IsInVehicle())
	{
		if (NewMovementMode != MOVE_Custom || NewCustomMode != 7)
		{
			SetMovementMode(MOVE_Custom, 7);
			NativeModeSwitchGuard_1dfc = 0;
			return;
		}
	}
	else
	{
		const EFortPawnStasisMode Stasis = Player->GetStasisMode();
		if ((Stasis == EFortPawnStasisMode::NoMovementOrFalling || Stasis == EFortPawnStasisMode::NoMovementOrFalling_EmotesEnabled)
			&& !Player->bLocalIsSkydiving && NewMovementMode != MOVE_None)
		{
			NativeModeSwitchGuard_1dfc = 0;
			return;
		}

		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			const FGameplayTag& StasisTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x7a8);
			if (ASC->HasMatchingGameplayTag(StasisTag))
			{
				Super::SetMovementMode(NewMovementMode, NewCustomMode);
				NativeModeSwitchGuard_1dfc = 0;
				return;
			}
		}

		if (Player->bIsSkydiving)
		{
			if (NewMovementMode == MOVE_Custom && (NewCustomMode == 4 || NewCustomMode == 5))
			{
				NewCustomMode = 3;
			}
		}
		else if (Player->bIsInVortex || Player->CurrentSkyTube != nullptr)
		{
			if (NewMovementMode != MOVE_Custom || NewCustomMode < 1 || NewCustomMode > 5)
			{
				Player->Native_1423320a0();
				NativeModeSwitchGuard_1dfc = 0;
				return;
			}
		}
		else
		{
			if (Player->GetNumActiveBalloons() > 0)
			{
				if (NativeAirborneFlag != 0 && NewMovementMode == MOVE_Falling)
				{
					NewMovementMode = MOVE_Custom;
					NewCustomMode = 9;
				}
			}
			else if (Player->IsZiplining() && NewMovementMode == MOVE_Falling)
			{
				NewMovementMode = MOVE_Custom;
				NewCustomMode = 8;
			}
		}
	}

	NativeModeSwitchGuard_1dfc = 0;
}

Super::SetMovementMode(NewMovementMode, NewCustomMode);
