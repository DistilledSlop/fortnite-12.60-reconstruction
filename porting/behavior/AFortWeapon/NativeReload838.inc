// CONFIDENCE: HIGH. Original0x1426aecd0; dynamic weapon +0x838.
// +838 ranged0x1426dd9e0 is a direct thunk to this exact body.
if (!IsEquipped()) return;
AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator());
if (!ReloadAbilitySpecHandle.IsValid()) return; // Shared engine IsValid is Handle != INDEX_NONE.
AController* Controller = NativeResolveController1426a07c0();
if (Controller)
{
    // Original redundant AController cast precedes this block; resolver returns AController*.
    IFortInventoryOwnerInterface* InventoryOwner = Cast<IFortInventoryOwnerInterface>(Controller);
    if (InventoryOwner) InventoryOwner->NativeNotifyReload78(this);
    if (AFortPlayerController* PC = Cast<AFortPlayerController>(Controller))
        PC->NativeCancelSprint1423b8500();
}
LastReloadTime = GetWorld()->TimeSeconds;
NativeAbilityEnd758(); // Dynamic +758, before the fresh pawn ASC/handle reads.
UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
if (ASC && ASC->TryActivateAbility(ReloadAbilitySpecHandle, true))
{
    float Duration = GetReloadTime(); // virtual +860 already canonical14269e880.
    const FFortBaseWeaponStats* Stats = NativeGetWeaponStats(); // virtual +6a8.
    const EFortWeaponReloadType ReloadType = Stats ? Stats->ReloadType : EFortWeaponReloadType::ReloadWholeClip;
    if (Stats && static_cast<uint8>(ReloadType) != 0)
    {
        const int32 Capacity = NativeGetBulletsPerClip14269b000();
        if (Capacity > 0 && NativeRecoveredInt32_0x8d8 < Capacity)
            Duration *= static_cast<float>(static_cast<int32>(static_cast<uint32>(Capacity) - static_cast<uint32>(NativeRecoveredInt32_0x8d8))) / static_cast<float>(Capacity);
    }
    OnLocalReloadStarted.Broadcast(Duration, ReloadType);
    if (NativeSupportsCharge1426a5af0() && NativeIsAutomatic1426a5ab0())
    {
        NativeChargeReload900Bit7 = true; // Original dword[900] |= 0x80.
        NativeStartCharge142698da0();
    }
}
