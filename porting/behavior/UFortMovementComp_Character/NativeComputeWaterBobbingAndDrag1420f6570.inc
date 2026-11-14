// Original address: 0x1420f6570
// CONFIDENCE: high
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
const float NormDepth = NativeWaterNormDepth_1d0c;
const float Depth = NativeRecoveredFloat_0x1d08;
const float HalfHeight = (Player && Player->GetCapsuleComponent()) ? Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
const float CapsuleZ = (Player && Player->GetCapsuleComponent()) ? Player->GetCapsuleComponent()->GetComponentLocation().Z : 0.0f;
const float TargetDepth = (Player && Player->bIsDBNO ? SurfaceSwimmingWaterControlParams.DBNOSwimImmersionDepth : SurfaceSwimmingWaterControlParams.TargetImmersionDepth).GetValueAtLevel(0.0f);
const float SurfaceZ = NativeRecoveredFVector_0x1d3c.Z;

const float GravZ = GetGravityZ();
const float Scale = FMath::Clamp(TargetDepth * (0.5f / FMath::Max(HalfHeight, 0.0001f)), 0.0001f, 1.0f);
float BuoyancyAccel = (1.0f - (NormDepth / Scale)) * GravZ;
const float MaxForce = SurfaceSwimmingWaterControlParams.BobbingMaxForce.GetValueAtLevel(0.0f);
BuoyancyAccel = FMath::Clamp(BuoyancyAccel, -MaxForce, MaxForce);

const bool bWaterJump = Player && Player->bIsWaterJump;
if (bWaterJump && BuoyancyAccel <= 0.0f)
{
    BuoyancyAccel = 0.0f;
}

Velocity.Z += BuoyancyAccel * DeltaTime;

if (!bWaterJump && FMath::Abs(Velocity.Z) > 0.1f)
{
    const float IdealTol = SurfaceSwimmingWaterControlParams.BobbingIdealDepthTolerance.GetValueAtLevel(0.0f);
    const float VelSign = (Velocity.Z > 0.0f) ? 1.0f : ((Velocity.Z < 0.0f) ? -1.0f : 0.0f);
    const float AccelSign = (BuoyancyAccel > 0.0f) ? 1.0f : ((BuoyancyAccel < 0.0f) ? -1.0f : 0.0f);
    if (VelSign != AccelSign || FMath::Abs(Depth - TargetDepth) <= IdealTol)
    {
        float FrictionVal = 0.0f;
        float ExpDragVal = 0.0f;
        if (Velocity.Z <= 0.0f)
        {
            const bool bSubmerged = (Depth >= HalfHeight * 2.5f);
            FrictionVal = (bSubmerged ? SurfaceSwimmingWaterControlParams.BobbingFrictionDownSubmerged : SurfaceSwimmingWaterControlParams.BobbingFrictionDown).GetValueAtLevel(0.0f);
            ExpDragVal = (bSubmerged ? SurfaceSwimmingWaterControlParams.BobbingExpDragDownSubmerged : SurfaceSwimmingWaterControlParams.BobbingExpDragDown).GetValueAtLevel(0.0f);
        }
        else
        {
            FrictionVal = SurfaceSwimmingWaterControlParams.BobbingFrictionUp.GetValueAtLevel(0.0f);
            ExpDragVal = SurfaceSwimmingWaterControlParams.BobbingExpDragUp.GetValueAtLevel(0.0f);
            if (Player && Player->bIsWaterSprintBoost)
            {
                const float BoostMult = FMath::Max(SurfaceSwimmingWaterControlParams.BoostDragMultiplier.GetValueAtLevel(0.0f), 1.0f);
                FrictionVal *= BoostMult;
                ExpDragVal *= FMath::Sqrt(BoostMult);
            }
        }
        const float LinDecay = FMath::Min(FrictionVal * DeltaTime, 1.0f);
        const float AfterLin = Velocity.Z * (1.0f - LinDecay);
        const float ExpDecay = FMath::Min(FMath::Abs(AfterLin) * ExpDragVal * DeltaTime * ExpDragVal, 1.0f);
        Velocity.Z = AfterLin * (1.0f - ExpDecay);
    }
}

return SurfaceZ - (TargetDepth + (CapsuleZ - HalfHeight));
