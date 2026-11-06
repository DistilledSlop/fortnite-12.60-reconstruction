// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Native 0x14165abf0 normalizes the vector before converting it to a yaw.
const float Yaw = DirectionVector.GetSafeNormal().Rotation().Yaw;
return static_cast<EFortEncounterDirection::Type>(FMath::TruncToInt(Yaw * 0.022222223f + 8.5f) % 8);
