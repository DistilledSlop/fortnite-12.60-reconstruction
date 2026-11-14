// CONFIDENCE: HIGH. Complete body; shared native state is declared in field proposals.
if (!WeaponData)
    return nullptr;
if (bUseAthenaRecoil && NativeOverrideWeaponStats201)
    return NativeOverrideWeaponStats201;
return WeaponData->WeaponStatHandle.GetRow<FFortBaseWeaponStats>(
    TEXT("UFortWeaponItemDefinition::GetWeaponStats"));
