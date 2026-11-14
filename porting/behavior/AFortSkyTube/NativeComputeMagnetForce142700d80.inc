// Original address: 0x142700d80
// CONFIDENCE: high
const FVector ToCenter = SplineLocation - CurrentLocation;
const float DistSq = ToCenter.SizeSquared();
const FVector RadialDir = ToCenter.GetSafeNormal();
const FVector SteeringDir = SteeringInput.GetSafeNormal();
const bool bHasSteering =
    FMath::Abs(SteeringDir.X) > 0.0001f ||
    FMath::Abs(SteeringDir.Y) > 0.0001f ||
    FMath::Abs(SteeringDir.Z) > 0.0001f;
float AlignScale = bHasSteering ? FMath::Abs(FVector::DotProduct(SplineDirection, SteeringDir)) : 1.0f;
if (AlignScale < NativeSteeringMagnetThreshold_0538)
{
    AlignScale = 0.0f;
}

FVector OutForce = FVector::ZeroVector;
const float InnerRadius = InnerMagnetRadius.GetValueAtLevel(0.0f);
if (DistSq <= InnerRadius * InnerRadius)
{
    const float AxialSpeed = FVector::DotProduct(Velocity, SplineDirection);
    const FVector RadialVel = Velocity - SplineDirection * AxialSpeed;
    const float InnerSpeedThresh = InnerMagnetSpeedThreshold.GetValueAtLevel(0.0f);
    const float RadialVelSq = RadialVel.SizeSquared();
    if (RadialVelSq > InnerSpeedThresh * InnerSpeedThresh)
    {
        OutForce = -RadialVel.GetSafeNormal() * (DeltaTime * InnerMagnetDecel.GetValueAtLevel(0.0f));
        const float RadialVelSize = FMath::Sqrt(RadialVelSq);
        if (RadialVelSize >= 0.0001f)
        {
            if (OutForce.SizeSquared() > RadialVelSq)
            {
                OutForce = OutForce.GetSafeNormal() * RadialVelSize;
            }
        }
        else
        {
            OutForce = FVector::ZeroVector;
        }
    }
    const float PowExp = FMath::Max(SteeringReduceDecelerationPower.GetValueAtLevel(0.0f), 1.0f);
    AlignScale = FMath::Pow(AlignScale, PowExp);
}
else
{
    const float RadialSpeed = FVector::DotProduct(Velocity, RadialDir);
    const FVector RadialComp = RadialDir * RadialSpeed;
    const float OuterSpeedThresh = OuterMagnetSpeedThreshold.GetValueAtLevel(0.0f);
    if (FVector::DotProduct(RadialDir, RadialComp) <= 0.0f ||
        RadialComp.SizeSquared() < OuterSpeedThresh * OuterSpeedThresh)
    {
        OutForce = RadialDir * (OuterMagnetAccel.GetValueAtLevel(0.0f) * DeltaTime);
        const float PowExp = FMath::Max(SteeringReduceAccelerationPower.GetValueAtLevel(0.0f), 1.0f);
        AlignScale = FMath::Pow(AlignScale, PowExp);
    }
    const float LowVelThresh = MagnetLowVelocityThreshold.GetValueAtLevel(0.0f);
    if (Velocity.SizeSquared() <= LowVelThresh * LowVelThresh)
    {
        const float LowVelMin = MagnetLowVelocityMinValue.GetValueAtLevel(0.0f);
        if (AlignScale < LowVelMin)
        {
            AlignScale = LowVelMin;
        }
    }
}

if (bHasSteering)
{
    const float MaxMult = MagnetMaxMultiplierWhenSteering.GetValueAtLevel(0.0f);
    if (MaxMult < AlignScale)
    {
        AlignScale = MaxMult;
    }
}
return OutForce * AlignScale;
