// Behaviour snippet: UBuildingStructuralSupportSystem::IsWorldLocValid (body only; spliced into the generated member function).
// Original body address: 0x141c825b0 (thunk 0x14275cb40)
// Offset evidence: per-axis lower bounds at this+0x384 (X), +0x388 (Y), +0x38c (Z); upper bounds at +0x390, +0x394, +0x398. No reflected member covers 0x378-0x398 (own-region gap, NativeWorldBounds_0x384 FBox).
// Tolerance 1.0e-4f from .rdata 0x1459c03a4. Comparisons mirror the asm COMISS/JA/JBE ordering, so an unordered (NaN) compare passes the same way the asm does.
// CONFIDENCE: HIGH
const float Epsilon = 9.999999747378752e-05f;
const float X = WorldLoc.X;
const float Y = WorldLoc.Y;
const float Z = WorldLoc.Z;
const bool bXLowOk = (X > NativeWorldBounds_0x384.Min.X) || !(FMath::Abs(X - NativeWorldBounds_0x384.Min.X) > Epsilon);
const bool bXHighOk = !(X > NativeWorldBounds_0x384.Max.X) || !(FMath::Abs(X - NativeWorldBounds_0x384.Max.X) > Epsilon);
const bool bYLowOk = (Y > NativeWorldBounds_0x384.Min.Y) || !(FMath::Abs(Y - NativeWorldBounds_0x384.Min.Y) > Epsilon);
const bool bYHighOk = !(Y > NativeWorldBounds_0x384.Max.Y) || !(FMath::Abs(Y - NativeWorldBounds_0x384.Max.Y) > Epsilon);
const bool bZLowOk = (Z > NativeWorldBounds_0x384.Min.Z) || !(FMath::Abs(Z - NativeWorldBounds_0x384.Min.Z) > Epsilon);
const bool bZHighOk = !(Z > NativeWorldBounds_0x384.Max.Z) || !(FMath::Abs(Z - NativeWorldBounds_0x384.Max.Z) > Epsilon);
return bXLowOk && bXHighOk && bYLowOk && bYHighOk && bZLowOk && bZHighOk;
