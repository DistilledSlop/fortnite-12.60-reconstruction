// CONFIDENCE: HIGH. Original 1418bfe90.
if (NativeDisableSkydivingFeedback14810e720() != 0)
    {
        NativeFeedbackChanged.Broadcast(uint8(3));
        return;
    }
    NativeFeedbackChanged.Broadcast(State);
    if (State == 0)
    {
        APlayerController* Owner = Cast<APlayerController>(GetOwner());
        if (!Owner || !Owner->PlayerCameraManager)
            return;
        SkydiveCamShake = Owner->PlayerCameraManager->PlayCameraShake(
            SkydiveCamShakeClass, 1.0f, ECameraAnimPlaySpace::CameraLocal, FRotator::ZeroRotator);
        AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Owner->GetPawn());
        if (!Pawn)
            return;
        UFortMovementComp_Character* Movement = Cast<UFortMovementComp_Character>(Pawn->GetCharacterMovement());
        if (!Movement)
            return;
        CachedSkydiveDropSpeedMax = Movement->SkydivingControlParamsActive.TerminalVelocity.GetValue();
        CachedSkydiveDropSpeedMin = Movement->SkydivingControlParamsPassive.TerminalVelocity.GetValue();
    }
    else if (SkydiveCamShake && (State == 1 || State == 3))
    {
        APlayerController* Owner = NativeGetPlayerController1418b2820();
        if (Owner && Owner->PlayerCameraManager)
            Owner->PlayerCameraManager->StopCameraShake(SkydiveCamShake, false);
    }
