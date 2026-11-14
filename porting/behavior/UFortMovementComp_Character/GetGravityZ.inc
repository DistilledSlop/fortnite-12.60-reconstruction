// Original address: 0x1420e0430
// CONFIDENCE: high
float BaseGrav = Super::GetGravityZ();
if (AFortPawn* FortPawn = Cast<AFortPawn>(PawnOwner))
{
    BaseGrav *= FortPawn->NativeGetGravityZScale14231c490();
    if (AFortPlayerPawn* FortPlayer = Cast<AFortPlayerPawn>(FortPawn))
    {
        BaseGrav *= FortPlayer->NativeGetVehicleGravityZScale142325690();
    }
}

const bool bAirCustom = (MovementMode == MOVE_Custom) &&
    (CustomMovementMode == 3 || CustomMovementMode == 4 || CustomMovementMode == 9 ||
     CustomMovementMode == 5 || CustomMovementMode == 12);
if (!bAirCustom)
{
    return BaseGrav;
}

const float Level = GetAirControlLevel();
const FAirControlParams& ActiveParams = GetAirControlParams();
float GravScalar = ActiveParams.GravityScalar.GetValueAtLevel(Level);

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
const bool bInVortex = Player && Player->bIsInVortex;
bool bInSkyTube = false;
if (Player && Player->CurrentSkyTube != nullptr)
{
    bInSkyTube = true;
    if (NativeShouldUseSkyTubeActiveAirParams1420ef2b0())
    {
        GravScalar = Player->CurrentSkyTube->ExitGravity.GetValueAtLevel(0.0f);
    }
}

if (bInVortex || bInSkyTube)
{
    if (NativeAirDirection_1420e4ab0())
    {
        const float FallDirZ = NativeGetCurrentFallDirectionZ1420e2940();
        const FAirControlParams& PassiveParams = bInSkyTube ? SkyTubeSkydivingControlParamsPassive
            : (bInVortex ? VortexSkydivingControlParamsPassive : SkydivingControlParamsPassive);
        const float PassiveScalar = PassiveParams.GravityScalar.GetValueAtLevel(Level);
        GravScalar = FMath::Lerp(PassiveScalar, GravScalar, FallDirZ);
    }
    else if (NativeAirDirection_1420e4740())
    {
        const float FallDirZ = NativeGetCurrentFallDirectionZ1420e2940();
        const float ParaScalar = NativeGetParachuteAirControlParams1420dfab0().GravityScalar.GetValueAtLevel(Level);
        GravScalar = FMath::Lerp(ParaScalar, GravScalar, FallDirZ);
    }
}

if (bInVortex && !bInSkyTube)
{
    if (AFortPlayerPawn* VortexPawn = Cast<AFortPlayerPawn>(PawnOwner))
    {
        const float Width = FMath::Abs(VortexPawn->VortexParams.GravityFloorWidth);
        const float TopZ = Width + VortexPawn->VortexParams.GravityFloorAltitude;
        const float LocZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
        const float SnapZ = FMath::GridSnap(LocZ, 10.0f);
        if (SnapZ <= TopZ)
        {
            const float Alpha = (Width > 1e-8f)
                ? FMath::Clamp((SnapZ - VortexPawn->VortexParams.GravityFloorAltitude) / Width, 0.0f, 1.0f)
                : (SnapZ >= TopZ ? 1.0f : 0.0f);
            GravScalar = FMath::Lerp(VortexPawn->VortexParams.GravityFloorGravityScalar, GravScalar, Alpha);
        }
    }
}

if (MovementMode == MOVE_Custom && CustomMovementMode == 9 && (GravScalar < 0.0f || Velocity.Z > 0.0f))
{
    float MinZ = 0.0f;
    float CenterZ = 0.0f;
    float MaxZ = 0.0f;
    if (NativeGetVortexCeilingRange1420ddab0(ActiveParams, Level, MinZ, CenterZ, MaxZ))
    {
        const float LocZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
        const float SnapZ = FMath::GridSnap(LocZ, 10.0f);
        if (SnapZ > MinZ)
        {
            if (SnapZ < CenterZ)
            {
                const float Alpha = (FMath::Abs(CenterZ - MinZ) > 1e-8f)
                    ? FMath::Clamp((SnapZ - MinZ) / (CenterZ - MinZ), 0.0f, 1.0f)
                    : (SnapZ >= CenterZ ? 1.0f : 0.0f);
                GravScalar = GravScalar - GravScalar * Alpha;
            }
            else
            {
                const float UpBoost = (Velocity.Z > 0.0f && SnapZ < MaxZ) ? 4.0f : 1.0f;
                const float AbsScalar = FMath::Max(FMath::Abs(GravScalar), 0.1f);
                const float Alpha = (FMath::Abs(MaxZ - CenterZ) > 1e-8f)
                    ? FMath::Clamp((SnapZ - CenterZ) / (MaxZ - CenterZ), 0.0f, 1.0f)
                    : (SnapZ >= MaxZ ? 1.0f : 0.0f);
                GravScalar = AbsScalar * UpBoost * Alpha;
            }
        }
    }
}

return BaseGrav * GravScalar;
