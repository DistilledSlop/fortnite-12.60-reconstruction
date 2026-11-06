// CONFIDENCE: high
// 0x141c6a510. The membership set is checked before geometric containment.
QUICK_SCOPE_CYCLE_COUNTER(STAT_Is_Actor_Within_Volume);
if (!Actor)
{
    return false;
}
const bool bPointBounds = !(Bounds.BoxExtent.Size() > 1.e-8f || FMath::Abs(Bounds.SphereRadius) > 1.e-8f);
if (NativeActorMembership.Contains(TWeakObjectPtr<AActor>(Actor)))
{
    return true;
}
if (bPointBounds || !bUseBounds)
{
    if (!Actor->IsA<AInfo>())
    {
        return NativeContainsLocation(Actor->GetActorLocation());
    }
}
else if (BoundsShape == EVolumeShape::Box)
{
    const FBox Box = NativeGetBoundsBox();
    const FVector Minimum = Bounds.Origin - Bounds.BoxExtent;
    const FVector Maximum = Bounds.Origin + Bounds.BoxExtent;
    const auto ContainsCorner = [&Box](const FVector& Point)
    {
        return !(Point.X < Box.Min.X - 175.f || Point.X > Box.Max.X + 175.f ||
            Point.Y < Box.Min.Y - 175.f || Point.Y > Box.Max.Y + 175.f ||
            Point.Z < Box.Min.Z - 175.f || Point.Z > Box.Max.Z + 175.f);
    };
    return ContainsCorner(Minimum) && ContainsCorner(Maximum);
}
else if (BoundsShape == EVolumeShape::Sphere)
{
    FBoxSphereBounds VolumeBounds;
    NativeGetLocationBounds(VolumeBounds);
    if (Bounds.SphereRadius <= VolumeBounds.SphereRadius + 0.0001f)
    {
        const float Radius = VolumeBounds.SphereRadius + 0.0001f - Bounds.SphereRadius;
        return (Bounds.Origin - VolumeBounds.Origin).SizeSquared() <= Radius * Radius;
    }
}
return false;
