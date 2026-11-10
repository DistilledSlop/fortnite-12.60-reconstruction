// CONFIDENCE: MEDIUM; thunk 0x142a7f410 calls impl 0x14234a5e0 (impl is void; the thunk's ReturnValue is the register left by the impl, LOW). Impl: re-entrancy guard (byte this+0x1610 bit 0x4), Role==Authority gate, VehicleStateLocal.Vehicle -> GetInterfaceAddress(UFortVehicleInterface) (0x142ec9900 / 0x142b2ca60), TryExitVehicle via interface vcall 0x68 (AFortAthenaVehicle::TryExitVehicle per log text), quat->rotator 0x142c5d000 (FQuat::Rotator), exit-location search 0x143f83570 (sub-helper 0x14411b9f0 not read), CharacterMovement SetMovementMode(MOVE_Falling) 0x1420dcab0, capsule flag 0x144124090, then removal of the two in-vehicle gameplay tags (native tags +0x1590, +0x1598) from the ability provider's ASC via 0x1406d8500, and guard clear.
if (bIsExitingVehicleGuard)
{
	return;
}
bIsExitingVehicleGuard = true;
if (Role == ROLE_Authority)
{
	if (!VehicleStateLocal.Vehicle)
	{
		// Log: tried to exit vehicle, but local vehicle state pointer was null (verbosity gated).
	}
	else
	{
		IFortVehicleInterface* VehicleInterface = Cast<IFortVehicleInterface>(VehicleStateLocal.Vehicle->GetInterfaceAddress(UFortVehicleInterface::StaticClass()));   // 0x142ec9900 / 0x142b2ca60
		if (!VehicleInterface)
		{
			// Log: same message as the null-vehicle-state case.
		}
		else
		{
			ExitTransform ExitOut = {};
			const bool bExited = VehicleInterface->TryExitVehicle(this, ExitOut, ExitForceBehavior);   // vcall 0x68
			if (!bExited)
			{
				// Log: AFortAthenaVehicle::TryExitVehicle returned false (verbosity gated).
			}
			else
			{
				FRotator ExitRotation = ExitOut.Rotation.Quaternion().Rotator();   // FUN_142c5d000 (FQuat::Rotator)
				TryFindExitTransform(this, ExitOut.Location, ExitRotation, false);   // FUN_143f83570 (sub-helper 0x14411b9f0 not read)
				if (!CharacterMovement)
				{
					// Log: tried to end vehicle driving movement, but movement component was null (verbosity gated).
				}
				else if (VehicleInterface->GetDrivingMovementComponent() == this)   // vcall 0x118 on the vehicle interface
				{
					CharacterMovement->SetMovementMode(MOVE_Falling);   // FUN_1420dcab0 (vcall 0x590 with 3)
				}
				else
				{
					SetCapsuleNativeFlag(CapsuleComponent, false);   // FUN_144124090 (flag bit 3 of +0x14c; argument register not verified)
					CharacterMovement->SetMovementMode(MOVE_Falling);
				}
				ClearExitVehicleOverride(this, 0, 0, true);   // vcall 0x758
			}
		}
	}
	UAbilitySystemComponent* ASC = AbilityProvider.GetAbilitySystemComponent();   // vcall 0x10 on embedded object at +0x4c8
	if (!ASC)
	{
		// Log: tried to remove in-vehicle gameplay tag, but ASC was null (verbosity gated).
	}
	else
	{
		FGameplayTagContainer InVehicleTags;
		InVehicleTags.AddTag(NativeGameplayTags()->InVehicleTagA);   // +0x1590
		InVehicleTags.AddTag(NativeGameplayTags()->InVehicleTagB);   // +0x1598
		RemoveGameplayTagContainerFromASC(ASC, InVehicleTags);   // FUN_1406d8500
	}
}
bIsExitingVehicleGuard = false;
