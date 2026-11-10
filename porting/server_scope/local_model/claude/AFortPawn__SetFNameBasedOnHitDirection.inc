// CONFIDENCE: HIGH
// Original: exec thunk 0x142a1f9d0; native body 0x14234c7b0; angle helper 0x142c521d0 (inlined below).
// FFrame read order: HitDirection, NameResult (out), NameFront, NameBack, NameLeft, NameRight.
// Frame axes come from RootComponent's ComponentToWorld rotation (+0x1c0); identity when there is no root component.
// The helper also computes an asin pitch that the caller never reads, so it is omitted (no side effects).
const FQuat Rot = RootComponent ? RootComponent->GetComponentQuat() : FQuat(0.f, 0.f, 0.f, 1.f);
const FVector Forward = Rot.RotateVector(FVector(1.f, 0.f, 0.f));
const FVector Right = Rot.RotateVector(FVector(0.f, 1.f, 0.f));
const FVector Up = Rot.RotateVector(FVector(0.f, 0.f, 1.f));

// Normalise the hit direction; degenerate input becomes the zero vector (helper behaviour).
FVector Dir = HitDirection;
const float DirSq = Dir.SizeSquared();
if (DirSq != 1.f)
{
    Dir = (DirSq >= 1e-8f) ? Dir / FMath::Sqrt(DirSq) : FVector::ZeroVector;
}

// Project onto the plane perpendicular to Up and normalise again.
const float UpDot = FVector::DotProduct(Dir, Up);
FVector Projected = Dir - Up * UpDot;
const float ProjSq = Projected.SizeSquared();
if (ProjSq != 1.f)
{
    Projected = (ProjSq >= 1e-8f) ? Projected / FMath::Sqrt(ProjSq) : FVector::ZeroVector;
}

// Signed yaw from Forward; the sign comes from Right (>= 0 is positive).
const float SideSign = (FVector::DotProduct(Projected, Right) >= 0.f) ? 1.f : -1.f;
const float YawRad = FMath::Acos(FMath::Clamp(FVector::DotProduct(Projected, Forward), -1.f, 1.f)) * SideSign;
const float YawDeg = YawRad * 57.295776f;
const float AbsYawDeg = FMath::Abs(YawDeg);

if (AbsYawDeg > 45.f)
{
    if (AbsYawDeg < 120.f)
    {
        NameResult = (YawDeg > 0.f) ? NameRight : NameLeft;
    }
    else
    {
        NameResult = NameBack;
    }
}
else
{
    NameResult = NameFront;
}
