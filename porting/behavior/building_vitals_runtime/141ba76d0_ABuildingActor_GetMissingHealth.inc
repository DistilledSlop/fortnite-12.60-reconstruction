// Original address: 0x141ba76d0.
// CONFIDENCE: HIGH
// Direct callee; see building_vitals_runtime/README.md.
const float Missing = GetMaxHealth() - GetHealth();
return (Missing > 0.0f) ? Missing : 0.0f;
