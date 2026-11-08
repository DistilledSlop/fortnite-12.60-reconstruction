// Original body 0x14269adb0 (thunk 0x142b45b40): CALL [vtable+0x6a8] = AFortWeapon::NativeGetWeaponStats on this; null -> EAX=1; else tail-jump 0x142668ff0 = UFortWeaponItemDefinition::NativeAmmoCostForCharge(*Stats, ChargeFraction) with XMM1 = charge
// CONFIDENCE: HIGH
static_assert(sizeof(FFortBaseWeaponStats) > 0, "FFortBaseWeaponStats must be complete for the dereference");
FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
if (Stats == nullptr)
{
    return 1;
}
return UFortWeaponItemDefinition::NativeAmmoCostForCharge(*Stats, ChargePercentage);
