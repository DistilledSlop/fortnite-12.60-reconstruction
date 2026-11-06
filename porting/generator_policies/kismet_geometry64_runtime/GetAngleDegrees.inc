// CONFIDENCE: HIGH
// Native 0x141ef0300: the initial unordered comparison retains NaN vectors.
auto Normalize = [](const FVector& V) -> FVector
{
    const float SquareSum = V.X * V.X + V.Y * V.Y + V.Z * V.Z;
    if (!(SquareSum < 1.0f) && !(SquareSum > 1.0f)) return V;
    if (SquareSum >= 1.0e-8f)
    {
        const float Scale = FMath::InvSqrt(SquareSum);
        return FVector(V.X * Scale, V.Y * Scale, V.Z * Scale);
    }
    return FVector::ZeroVector;
};
const FVector A = Normalize(Source);
const FVector B = Normalize(Target);
const float Dot = B.Y * A.Y + B.X * A.X + B.Z * A.Z;
const float Clamped = !(Dot >= -1.0f) ? -1.0f : (Dot >= 1.0f ? 1.0f : Dot);
return FMath::Acos(Clamped) * 57.295776f;
