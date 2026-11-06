// CONFIDENCE: HIGH
// 0x142668ff0; ordered comparisons map NaN and negative input to zero.
float Fraction = 0.0f;
if (ChargeFraction >= 0.0f)
    Fraction = ChargeFraction < 1.0f ? ChargeFraction : 1.0f;
const int32 MinCost = FMath::Max(Stats.AmmoCostPerFire, 1);
const int32 MaxCost = FMath::Max(Stats.MaxAmmoCostPerFire, 1);
// Unsigned arithmetic preserves the native 32-bit subtraction.
const uint32 DifferenceBits = uint32(MaxCost) - uint32(MinCost);
int32 Difference;
FMemory::Memcpy(&Difference, &DifferenceBits, sizeof(Difference));
return FPlatformMath::TruncToInt(float(Difference) * Fraction + float(MinCost));
