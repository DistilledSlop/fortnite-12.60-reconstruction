// CONFIDENCE: HIGH
// Original UFortMovementComp_Character::GetMaxAcceleration, 0x1420e1760.
const AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(CharacterOwner);
if (MovementMode == MOVE_Custom)
{
    if (CustomMovementMode == 3 || CustomMovementMode == 4 ||
        CustomMovementMode == 9 || CustomMovementMode == 5 || CustomMovementMode == 12)
        return GetAirControlParams().MaxAcceleration.GetValueAtLevel(GetAirControlLevel(), nullptr);

    if (CustomMovementMode == 10)
    {
        bool bSprinting = false;
        if (FortPawn)
        {
            uint8 MovementStyle;
            bSprinting = *FortPawn->NativeGetCurrentMovementStyle14231a980(&MovementStyle)
                == static_cast<uint8>(EFortMovementStyle::Sprinting);
        }
        const FScalableFloat& Acceleration = bSprinting
            ? SurfaceSwimmingWaterControlParams.MaxAccelerationSprinting
            : SurfaceSwimmingWaterControlParams.MaxAcceleration;
        return Acceleration.GetValueAtLevel(0.0f, nullptr);
    }

    if (CustomMovementMode == 6)
    {
        // Original directly dispatches through CharacterOwner's ability interface.
        const AFortPawn* AbilityOwner = static_cast<const AFortPawn*>(CharacterOwner);
        if (const UFortAbilitySystemComponent* Ability = Cast<UFortAbilitySystemComponent>(AbilityOwner->GetAbilitySystemComponent()))
            if (const UFortJumpBoostSet* Set = Ability->GetSet<UFortJumpBoostSet>())
                return Set->LateralAcceleration.GetCurrentValue();
    }
}

if (const AFortCharacterVehicle_Ostrich* Ostrich = Cast<AFortCharacterVehicle_Ostrich>(CharacterOwner))
{
    if (IsMovingOnGround())
    {
        const float Acceleration = Ostrich->bAcceleratingFromIdle ? Ostrich->AccelFromIdle : 0.0f;
        if (Acceleration > 0.0f)
            return Acceleration;
    }
}
return UCharacterMovementComponent::GetMaxAcceleration();
