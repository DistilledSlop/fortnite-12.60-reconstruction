// Body 0x142321eb0 (thunk 0x142a1c4a0). DecisionWindowStack +0xbc0 (reflected, 24-byte FFortActiveMontageDecisionWindow, Num +0xbc8). First entry with non-null DecisionWindow: PrimaryInputStrikeAngle (+0x60) when bPrimaryInput else SecondaryInputStrikeAngle (+0x64) (reflected on UFortAnimNotifyState_AbilityDecisionWindow).
// Fallback: CurrentWeapon +0x588 (reflected) tail-calls AFortPawn::NativeSelectMeleeReticleAngle14269eaa0 (0x14269eaa0); null weapon returns 0.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortAnimNotifyState_AbilityDecisionWindow) > 0, "UFortAnimNotifyState_AbilityDecisionWindow include for decision window fields");
static_assert(sizeof(AFortWeapon) > 0, "AFortWeapon include for CurrentWeapon helper call");
for (int32 i = 0; i < DecisionWindowStack.Num(); ++i)
{
    UFortAnimNotifyState_AbilityDecisionWindow* Window = DecisionWindowStack[i].DecisionWindow;
    if (Window)
    {
        return bPrimaryInput ? Window->PrimaryInputStrikeAngle : Window->SecondaryInputStrikeAngle;
    }
}
if (CurrentWeapon)
{
    return AFortPawn::NativeSelectMeleeReticleAngle14269eaa0(CurrentWeapon, bPrimaryInput);
}
return 0.0f;
