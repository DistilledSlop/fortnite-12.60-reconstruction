// CONFIDENCE: HIGH; see HANDOFF.md.
const FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
const int32 MaxCost = Stats ? UFortWeaponItemDefinition::NativeAmmoCostForCharge(*Stats, 1.0f) : 1;
const float Value = static_cast<float>(AmmoCount) / static_cast<float>(MaxCost);
return Value >= 0.0f ? (Value < 1.0f ? Value : 1.0f) : 0.0f;
