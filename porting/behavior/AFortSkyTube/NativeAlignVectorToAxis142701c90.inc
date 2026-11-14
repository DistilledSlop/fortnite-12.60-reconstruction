// Original address: 0x142701c90
// CONFIDENCE: high
const float Dot = FVector::DotProduct(Axis, InputVector);
FVector Result = (Axis * Dot).GetSafeNormal();
if (FMath::Abs(Result.X) <= 0.0001f && FMath::Abs(Result.Y) <= 0.0001f && FMath::Abs(Result.Z) <= 0.0001f)
{
    Result = Axis;
}
return Result;
