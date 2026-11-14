// CONFIDENCE: high
// CANDIDATE ONLY. Original 0x14231ab10, primary virtual +0x930.
const FFortPawnStats* Stats = NativeGetPawnStats141a07b70();
const FVector Forward = GetRootComponent() ? GetRootComponent()->GetForwardVector() : FVector::ForwardVector;
const FVector Direction = FortMovementSpeed64::Normal2D(GetCharacterMovement()->Velocity);
float Backwards = 1.0f;
if (!(FVector::DotProduct(Forward, Direction) >= -0.3f))
    Backwards = MovementSet ? MovementSet->BackwardSpeedMultiplier.GetCurrentValue() : Stats->SpeedBackwardsMultiplier;
float Result = 0.0f;
switch (Style)
{
case 0: Result = MovementSet ? MovementSet->RunSpeed.GetCurrentValue() * Backwards : Stats->SpeedRun; break;
case 1: case 4: Result = MovementSet ? MovementSet->WalkSpeed.GetCurrentValue() : Stats->SpeedWalk; break;
case 2: case 3: Result = MovementSet ? MovementSet->SprintSpeed.GetCurrentValue() * Backwards : Stats->SpeedSprint; break;
}
return FortMovementSpeed64::Positive(Result);
