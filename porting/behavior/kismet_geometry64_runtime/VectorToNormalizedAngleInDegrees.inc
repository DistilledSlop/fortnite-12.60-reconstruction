// CONFIDENCE: HIGH
// Native 0x141f0ac50 uses Unreal's Atan2 and NormalizeAxis implementations.
return FRotator::NormalizeAxis(FMath::Atan2(V.Y, V.X) * 57.295776f);
