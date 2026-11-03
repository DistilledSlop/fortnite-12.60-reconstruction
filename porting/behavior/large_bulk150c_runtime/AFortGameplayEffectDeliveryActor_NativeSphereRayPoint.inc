// Native 0x142c5ee10. CONFIDENCE: high
const FVector Delta = Center - Start;
const float B = -2.f * FVector::DotProduct(Delta, Direction);
const float Discriminant = B * B - 4.f * (Delta.SizeSquared() - Radius * Radius);
if (Discriminant > 0.0001f)
{
    const float Root = FMath::Sqrt(Discriminant);
    const float First = (-B - Root) * 0.5f;
    const float Second = Root * 0.5f - B * 0.5f;
    float Distance = FMath::Abs(Second);
    if (FMath::Abs(First) != FMath::Abs(Second))
    {
        Distance = FMath::Abs(First) <= FMath::Abs(Second) ? First : Second;
    }
    Point = Start + Direction * Distance;
}
else
{
    const FVector Radial = (Start + Direction * (B * -0.5f) - Center).GetSafeNormal();
    Point = Center + Radial * Radius;
}
