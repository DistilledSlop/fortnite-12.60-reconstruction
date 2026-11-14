// CONFIDENCE: HIGH
// Original interface slot +0x78 = 0x1416a6100; this is the owning actor.
// GetWeaponData preserves its native invalid-data diagnostic before +0x60 dispatch.
UFortWeaponItemDefinition* Definition = Weapon->GetWeaponData();
if (NativeHasInfiniteAmmo(Definition) && FortControllerComponent_Telemetry)
{
    FortControllerComponent_Telemetry->TotalAmmoCheats = static_cast<int32>(
        static_cast<uint32>(FortControllerComponent_Telemetry->TotalAmmoCheats) + 1u);
}
