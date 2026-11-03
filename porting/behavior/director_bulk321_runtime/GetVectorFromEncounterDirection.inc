// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Native 0x141660150 converts the direction's 45-degree yaw to a unit vector.
return FRotator(0.0f, static_cast<int32>(Direction) * 45.0f, 0.0f).Vector().GetSafeNormal();
