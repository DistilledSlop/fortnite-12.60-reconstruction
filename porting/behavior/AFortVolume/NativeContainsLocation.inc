// CONFIDENCE: HIGH
QUICK_SCOPE_CYCLE_COUNTER(STAT_Volume_Location_Lookup);
// 0x141c82210. Lower box comparisons intentionally reject unordered values.
if (BoundsShape == EVolumeShape::Box)
{
    FBoxSphereBounds Bounds;
    NativeGetLocationBounds(Bounds);
    return Location.X >= Bounds.Origin.X - Bounds.BoxExtent.X &&
           !(Location.X > Bounds.Origin.X + Bounds.BoxExtent.X) &&
           Location.Y >= Bounds.Origin.Y - Bounds.BoxExtent.Y &&
           !(Location.Y > Bounds.Origin.Y + Bounds.BoxExtent.Y) &&
           Location.Z >= Bounds.Origin.Z - Bounds.BoxExtent.Z &&
           !(Location.Z > Bounds.Origin.Z + Bounds.BoxExtent.Z);
}
if (BoundsShape == EVolumeShape::Sphere)
{
    FBoxSphereBounds Bounds;
    NativeGetLocationBounds(Bounds);
    const float DX = Bounds.Origin.X - Location.X;
    const float DY = Bounds.Origin.Y - Location.Y;
    const float DZ = Bounds.Origin.Z - Location.Z;
    const float Radius = Bounds.SphereRadius + 0.0001f;
    return Radius * Radius >= (DY * DY + DX * DX) + DZ * DZ;
}
return false;
