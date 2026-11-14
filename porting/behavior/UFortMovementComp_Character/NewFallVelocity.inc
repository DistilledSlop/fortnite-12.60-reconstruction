// Original address: 0x1420e5a40
// CONFIDENCE: high
FVector Vel = InitialVelocity;
const bool bAirCustom = (MovementMode == MOVE_Custom) &&
    (CustomMovementMode == 3 || CustomMovementMode == 4 || CustomMovementMode == 9 ||
     CustomMovementMode == 5 || CustomMovementMode == 12);

AFortPlayerPawn* CharPlayer = Cast<AFortPlayerPawn>(CharacterOwner);
if (bAirCustom && CharPlayer && CharPlayer->CurrentSkyTube != nullptr)
{
    Vel = NativeAirborneVelocity_1e0c;
}

if (!bAirCustom)
{
    return Super::NewFallVelocity(Vel, Gravity, DeltaTime);
}

const float GravZ = Gravity.Z;
const FAirControlParams& ActiveParams = GetAirControlParams();
const float Level = GetAirControlLevel();
const float DownTerm = ActiveParams.TerminalVelocity.GetValueAtLevel(Level);
const float UpTerm = ActiveParams.UpwardTerminalVelocity.GetValueAtLevel(Level);

float EffectiveTerm = 0.0f;
float DirSignZ = 0.0f;
if (Vel.Z < 0.0f)
{
    DirSignZ = -1.0f;
    EffectiveTerm = FMath::Abs(DownTerm);
}
else if (Vel.Z > 0.0f)
{
    DirSignZ = 1.0f;
    EffectiveTerm = FMath::Abs(UpTerm);
}

if (NativeAirDirection_1420e4ab0())
{
    const float FallDirZ = NativeGetCurrentFallDirectionZ1420e2940();
    const bool bInSkyTube = CharPlayer && CharPlayer->CurrentSkyTube != nullptr;
    const bool bInVortex = CharPlayer && CharPlayer->bIsInVortex;
    const FAirControlParams& PassiveParams = bInSkyTube ? SkyTubeSkydivingControlParamsPassive
        : (bInVortex ? VortexSkydivingControlParamsPassive : SkydivingControlParamsPassive);
    const float PassiveTerm = (Vel.Z > 0.0f ? PassiveParams.UpwardTerminalVelocity : PassiveParams.TerminalVelocity).GetValueAtLevel(Level);
    EffectiveTerm = FMath::Lerp(FMath::Abs(PassiveTerm), EffectiveTerm, FallDirZ);
}
else if (NativeAirDirection_1420e4740())
{
    const float FallDirZ = NativeGetCurrentFallDirectionZ1420e2940();
    const FAirControlParams& ParaParams = NativeGetParachuteAirControlParams1420dfab0();
    const float ParaTerm = (Vel.Z > 0.0f ? ParaParams.UpwardTerminalVelocity : ParaParams.TerminalVelocity).GetValueAtLevel(Level);
    EffectiveTerm = FMath::Lerp(FMath::Abs(ParaTerm), EffectiveTerm, FallDirZ);
}

if (CharPlayer && CharPlayer->CurrentSkyTube != nullptr)
{
    if (AFortPlayerPawn* PawnPlayer = Cast<AFortPlayerPawn>(PawnOwner))
    {
        if (AFortSkyTube* Tube = PawnPlayer->CurrentSkyTube)
        {
            const FVector CurLoc = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
            float InputKey = 0.0f;
            FVector SplineLoc = FVector::ZeroVector;
            FVector SplineDir = FVector::ZeroVector;
            Tube->NativeSampleSplineAtLocation142701530(CurLoc, InputKey, SplineLoc, SplineDir);
            if (!NativeShouldUseSkyTubeActiveAirParams1420ef2b0())
            {
                const FVector ForwardDir = NativeAirForward_1420e2860();
                const FVector& SteerInput = (NativeAirborneFlag != 0 || bAirCustom) ? CustomFallingAcceleration : Acceleration;
                const FVector MagnetForce = Tube->NativeComputeMagnetForce142700d80(SplineLoc, SplineDir, CurLoc, Vel, SteerInput, DeltaTime);
                const FVector AlignedVel = Tube->NativeAlignVectorWithSplineDirection142701880(SplineDir, Vel);
                FVector TangentialDelta = FVector::ZeroVector;
                if (Tube->NativeShouldReduceSteering142707a50(AlignedVel, ForwardDir, SteerInput))
                {
                    const float TangAccel = Tube->TangentialAcceleration.GetValueAtLevel(0.0f);
                    TangentialDelta = AlignedVel * (TangAccel * DeltaTime);
                }
                Vel = Vel + MagnetForce + TangentialDelta;
            }
            else
            {
                const float ExitAccel = Tube->ExitAcceleration.GetValueAtLevel(0.0f);
                if (Vel.SizeSquared2D() < ExitAccel * ExitAccel)
                {
                    const FVector RadialDir = (CurLoc - SplineLoc).GetSafeNormal();
                    Vel += RadialDir * (ExitAccel * DeltaTime);
                }
            }
        }
    }
}

float DownLimit = FMath::Abs(DownTerm);
if (Vel.Z <= 0.0f)
{
    if (NativeIsInVortex1420e53e0() && !NativeHasSkyTube_1420e5300())
    {
        if (AFortPlayerPawn* PawnPlayer = Cast<AFortPlayerPawn>(PawnOwner))
        {
            const float Width = FMath::Abs(PawnPlayer->VortexParams.GravityFloorWidth);
            const float TopZ = Width + PawnPlayer->VortexParams.GravityFloorAltitude;
            const float LocZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
            const float SnapZ = FMath::GridSnap(LocZ, 10.0f);
            if (SnapZ <= TopZ)
            {
                const float FloorTerm = FMath::Max(PawnPlayer->VortexParams.GravityFloorTerminalVelocity, 0.1f);
                const float Alpha = (Width > 1e-8f)
                    ? FMath::Clamp((SnapZ - PawnPlayer->VortexParams.GravityFloorAltitude) / Width, 0.0f, 1.0f)
                    : (SnapZ >= TopZ ? 1.0f : 0.0f);
                EffectiveTerm = FMath::Lerp(FloorTerm, EffectiveTerm, Alpha);
            }
        }
    }
    if (Vel.Z < 0.0f)
    {
        DownLimit = EffectiveTerm;
    }
}

const float UpLimit = (Vel.Z > 0.0f) ? EffectiveTerm : FMath::Abs(UpTerm);
float NewZ = Vel.Z;
if (FMath::Abs(NewZ) <= EffectiveTerm || EffectiveTerm == 0.0f)
{
    NewZ += GravZ * DeltaTime;
    if (NewZ < -DownLimit && DownLimit != 0.0f)
    {
        NewZ = -DownLimit;
    }
    else if (NewZ > UpLimit && UpLimit != 0.0f)
    {
        NewZ = UpLimit;
    }
}
else
{
    const float AccelOrZero = (DirSignZ * GravZ >= 0.0f) ? 0.0f : GravZ;
    const float DragFactor = FMath::Max(FMath::Abs(GravZ / EffectiveTerm), 0.5f);
    float RemainingDt = DeltaTime;
    while (RemainingDt >= 1e-6f)
    {
        float StepDt = RemainingDt;
        if (RemainingDt > (1.0f / 60.0f))
        {
            StepDt = FMath::Min(RemainingDt * 0.5f, 1.0f / 60.0f);
        }
        RemainingDt -= StepDt;
        NewZ += (AccelOrZero - DragFactor * NewZ) * StepDt;
        if (FMath::Abs(NewZ) <= EffectiveTerm || (DirSignZ * NewZ <= 0.0f))
        {
            NewZ = DirSignZ * EffectiveTerm;
            break;
        }
    }
}

Vel.Z = NewZ;
const_cast<UFortMovementComp_Character*>(this)->NativeAirborneVelocity_1e0c = Vel;
return Vel;
