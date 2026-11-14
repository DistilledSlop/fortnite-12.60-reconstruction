// CONFIDENCE: HIGH
// Original 0x1426cf820, primary ranged weapon vtable +0x6b0.
if (!WeaponData)
    return nullptr;
if (bUseAthenaRecoil && NativeOverrideWeaponStats201)
    return static_cast<FFortRangedWeaponStats*>(NativeOverrideWeaponStats201);
return WeaponData->WeaponStatHandle.GetRow<FFortRangedWeaponStats>(
    TEXT("UFortWeaponItemDefinition::GetWeaponStats"));
