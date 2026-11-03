// Original address: 0x141baf210.
// CONFIDENCE: HIGH
// Direct callee; see building_vitals_runtime/README.md.
const float MaxHealthValue = GetMaxHealth();
const float HealthValue = GetHealth();
return FMath::Abs(HealthValue - MaxHealthValue) > 0.01f;
