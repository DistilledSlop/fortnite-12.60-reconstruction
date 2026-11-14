// CONFIDENCE: HIGH
// Original 0x1426a5af0; complete ordered leaf.
const FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
UWorld* World = GetWorld();
return World && Stats && UFortWeaponItemDefinition::NativeSupportsCharge(*Stats);
