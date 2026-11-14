// CONFIDENCE: HIGH. Original0x1426dfd80; ranged override of original base1426b3730.
const bool bPreviousTargeting = bIsTargeting;
Super::NativeSetTargeting1426b3730(bNewTargeting);
if (bPreviousTargeting != bIsTargeting)
{
    NativeTargetingChangedCE8 = true;
}
// Original caller argument controls the timer predicate, independently of the post-base state.
if (bInitializedWeaponItem && !bNewTargeting && GetLocalMagazineAmmoCount() <= 0)
{
    const FFortRangedWeaponStats* Stats = NativeGetRangedWeaponStats6B0();
    float Delay = 0.4000999927520752f;
    // COMISS row,zero + JNC accepts >=0 and rejects unordered; Ghidra C loses the NaN case.
    if (Stats && Stats->AutoReloadDelayOverride >= 0.0f)
    {
        Delay = Stats->AutoReloadDelayOverride;
    }
    // Original141b8e130 is solely a tail jump to virtual848. Bind that virtual directly.
    GetWorldTimerManager().SetTimer(NativeAutoReloadTimerD98,
        FTimerDelegate::CreateUObject(static_cast<AFortWeapon*>(this), &AFortWeapon::NativeTryAutoReload848),
        Delay, false, -1.0f);
}
