// CONFIDENCE: HIGH. Original14230e850. Mode-change callbacks own the later state changes.
if (!bIsParachuteOpen) return;
UCharacterMovementComponent* Movement = GetCharacterMovement();
if (bLocalIsSkydiving)
{
    if (bIsSkydivingFromBus)
    {
        UFortControllerComponent_SkydiveFeedback* Feedback = NativeControllerFeedback1423240e0();
        if (Feedback) Feedback->NativeSetFeedback1418bfe90(0);
    }
    Movement->SetMovementMode(MOVE_Custom, 4);
}
else
{
    if (Movement->IsSwimming()) return;
    Movement->SetMovementMode(MOVE_Falling, 0);
}
