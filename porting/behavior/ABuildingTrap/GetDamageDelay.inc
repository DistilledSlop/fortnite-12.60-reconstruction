// Original target 0x141c39890; preserves virtual trap-data validity and base-row lookup.
// CONFIDENCE: HIGH
if (!NativeHasTrapData())
{
    return 0.0f;
}
const FFortBaseWeaponStats* BaseStats = TrapData->WeaponStatHandle.GetRow<FFortBaseWeaponStats>(
    TEXT("UFortWeaponItemDefinition::GetWeaponStats"));
return static_cast<const FFortTrapStats*>(BaseStats)->DamageDelay;
