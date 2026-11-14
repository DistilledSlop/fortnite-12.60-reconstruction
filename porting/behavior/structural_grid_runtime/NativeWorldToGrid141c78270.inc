// CONFIDENCE: HIGH
// bool NativeWorldToGrid141c78270(const FVector& World, FIntVector& Out) const
// Exact epsilon comparisons retain original NaN behavior; output unchanged on failure.
const FBox& Box = NativeWorldBounds_0x384;
auto InRange = [](float V, float Min, float Max) {
 return !((V <= Min && 0.0001f < FMath::Abs(V-Min)) ||
          (Max < V && 0.0001f < FMath::Abs(V-Max)));
};
if (!InRange(World.X, Box.Min.X, Box.Max.X) ||
    !InRange(World.Y, Box.Min.Y, Box.Max.Y) ||
    !InRange(World.Z, Box.Min.Z, Box.Max.Z)) return false;
const float X = (World.X - NativeRecoveredFloat_0x378) * 0.001953125f;
const float Y = (World.Y - NativeRecoveredFloat_0x37c) * 0.001953125f;
const float Z = (World.Z - NativeRecoveredFloat_0x380) * 0.0026041667f;
Out.X = FortStructuralOriginal::HalfConvert((X+X)+0.5f);
Out.Y = FortStructuralOriginal::HalfConvert((Y+Y)+0.5f);
Out.Z = FortStructuralOriginal::HalfConvert((Z+Z)+0.5f);
return true;
