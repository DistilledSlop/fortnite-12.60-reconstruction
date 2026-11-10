// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a2bad0; native body 0x14203cda0.
// FFrame read order: FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, bShouldCombinePickupsWhenTossCompletes,
// InPickupSourceTypeFlags, InPickupSpawnSource (the enum underlying bytes are not separate reads).
if (Role == ROLE_Authority) // byte +0xf0 == 3
{
    // Net dormancy: the flag byte at AActor+0x33 (0x143f7abb0, unnamed) selects flush vs. awake.
    if (!IsFlag33Set())
    {
        FlushNetDormancy(); // 0x143f72040 (engine AActor::FlushNetDormancy)
    }
    else
    {
        SetNetDormancy(DORM_Awake); // 0x143f84be0(this, 1)
    }

    bCombinePickupsWhenTossCompletes = bShouldCombinePickupsWhenTossCompletes; // +0x49c

    // Virtual slot 0x720 -> 0x142029370 (107-line body, NOT PORTED; see deps AFortPickup__TossPickupNative).
    TossPickupNative(FinalLocation, ItemOwner, OverrideMaxStackCount, bToss);

    // Owner controller path: only when the pawn's controller (APawn::Controller, +0x250) is a player controller.
    if (ItemOwner)
    {
        if (AFortPlayerController* OwnerPC = Cast<AFortPlayerController>(ItemOwner->Controller))
        {
            OnTossCompleteNotify();                 // 0x142038060 (deps, partly read)
            NotifyControllerOfToss(OwnerPC, 0);     // 0x1423bd380 (deps, not read; the 2nd argument is _DAT_14813b798)
        }
    }

    PickupSourceTypeFlags |= InPickupSourceTypeFlags; // byte +0x450 (OR)
    PickupSpawnSource = InPickupSpawnSource;          // byte +0x451
    ForceNetUpdate();                                 // vslot 0x5e8 -> AActor::ForceNetUpdate (0x143f72550)
}
