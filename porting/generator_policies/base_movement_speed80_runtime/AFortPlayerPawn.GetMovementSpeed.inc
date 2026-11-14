// CONFIDENCE: high
// CANDIDATE ONLY. Original 0x14231ada0, primary virtual +0x930.
const UFortMovementComp_Character* Movement = CastChecked<UFortMovementComp_Character>(GetCharacterMovement());
const FVector Direction = FortMovementSpeed64::Normal2D(Movement->Velocity);
const FVector Forward = GetRootComponent() ? GetRootComponent()->GetForwardVector() : FVector::ForwardVector;
const float ForwardDot = FVector::DotProduct(Forward, Direction);
const FFortPawnStats* Stats = NativeGetPawnStats141a07b70();
const float SpeedScale = MovementSet ? MovementSet->SpeedMultiplier.GetCurrentValue() : 1.0f;
if (bIsDBNO) return Stats->SpeedDBNO;
if (bMovingEmote) return EmoteWalkSpeed;
float Speed = 0.0f;
float DirectionScale = 1.0f;
const auto CurveAngle = [](float Dot) {
    // Original COMISS/JB selects -1 for an unordered dot product.
    float Clamped = -1.0f;
    if (Dot >= -1.0f) Clamped = Dot >= 1.0f ? 1.0f : Dot;
    return FMath::Acos(Clamped) * 57.295776f;
};
const bool bAir = Movement->MovementMode == MOVE_Custom &&
    (Movement->CustomMovementMode == 3 || Movement->CustomMovementMode == 4 ||
     Movement->CustomMovementMode == 9 || Movement->CustomMovementMode == 5 ||
     Movement->CustomMovementMode == 12);
const bool bSurfaceSwimming = Movement->MovementMode == MOVE_Custom && Movement->CustomMovementMode == 10;
if (bAir)
{
    Speed = Movement->GetAirControlParams().MaxLateralSpeed.GetValueAtLevel(0.0f);
    if (!CurrentSkyTube)
    {
        UCurveFloat* Curve = BallooningDirectionalSpeedMultiplierCurve;
        if (!Curve || Movement->MovementMode != MOVE_Custom || Movement->CustomMovementMode != 9)
            Curve = bIsParachuteOpen ? ParachuteDirectionalSpeedMultiplierCurve : SkydivingDirectionalSpeedMultiplierCurve;
        if (Curve) DirectionScale = Curve->GetFloatValue(CurveAngle(ForwardDot));
    }
}
else if (bSurfaceSwimming || bIsWaterJump)
{
    const FWaterControlParams& Water = Movement->SurfaceSwimmingWaterControlParams;
    if (!bIsWaterJump || bSurfaceSwimming)
    {
        if (Style == 1) Speed = Water.MaxTargetingSpeed.GetValueAtLevel(0.0f);
        else if (Style == 3)
            Speed = bIsWaterSprintBoost ? Movement->WaterSprintBoostParams.MaxSpeed.GetValueAtLevel(0.0f) : Water.MaxSprintSpeed.GetValueAtLevel(0.0f);
        else Speed = Water.MaxNormalSpeed.GetValueAtLevel(0.0f);
    }
    else
    {
        const float TargetingSpeed = Water.MaxTargetingSpeed.GetValueAtLevel(0.0f);
        const float CurrentSpeed = Movement->Velocity.Size2D();
        Speed = FortMovementSpeed64::MaxSecond(TargetingSpeed, CurrentSpeed);
    }
    if (!(ForwardDot >= Water.BackwardsSpeedCosAngle.GetValueAtLevel(0.0f)))
        DirectionScale = Water.BackwardsSpeedMultiplier.GetValueAtLevel(0.0f);
    else if (!FortMovementSpeed64::Zero(Direction) && Movement->GetCurrentAcceleration().SizeSquared2D() > 1.0f)
    {
        const FVector AccelerationDirection = FortMovementSpeed64::Normal2D(Movement->GetCurrentAcceleration());
        if (!(FVector::DotProduct(AccelerationDirection, Direction) >= Water.AngledSpeedCosAngle.GetValueAtLevel(0.0f)))
            DirectionScale = Water.AngledSpeedMultiplier.GetValueAtLevel(0.0f);
    }
}
else
{
    if (DirectionalSpeedMultiplierCurve)
        DirectionScale = DirectionalSpeedMultiplierCurve->GetFloatValue(CurveAngle(ForwardDot));
    else if (!(ForwardDot >= -0.3f))
        DirectionScale = MovementSet ? MovementSet->BackwardSpeedMultiplier.GetCurrentValue() : Stats->SpeedBackwardsMultiplier;
    switch (Style)
    {
    case 0:
        Speed = MovementSet ? (bIsCrouched ? MovementSet->CrouchedRunSpeed.GetCurrentValue() : MovementSet->RunSpeed.GetCurrentValue()) :
            (bIsCrouched ? Stats->SpeedCrouchedRun : Stats->SpeedRun);
        break;
    case 1:
        Speed = MovementSet ? MovementSet->WalkSpeed.GetCurrentValue() : Stats->SpeedWalk;
        DirectionScale = 1.0f;
        break;
    case 2: case 3:
        Speed = MovementSet ? (bIsCrouched ? MovementSet->CrouchedSprintSpeed.GetCurrentValue() : MovementSet->SprintSpeed.GetCurrentValue()) :
            (bIsCrouched ? Stats->SpeedCrouchedSprint : Stats->SpeedSprint);
        break;
    case 4: Speed = MovementSet ? MovementSet->WalkSpeed.GetCurrentValue() : Stats->SpeedWalk; break;
    case 5: Speed = MovementSet ? MovementSet->FlySpeed.GetCurrentValue() : Stats->SpeedFly; break;
    }
}
if (MovementSet && !(FMath::Abs(MovementSet->BackwardSpeedMultiplier.GetCurrentValue() - 1.0f) > 1.e-8f))
    DirectionScale = 1.0f;
float Result = DirectionScale * Speed * SpeedScale;
return FortMovementSpeed64::Positive(Result);
