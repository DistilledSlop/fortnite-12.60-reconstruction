// CONFIDENCE: HIGH. Original 0x142698da0; C and assembly preserve unordered comparison.
AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator());
if (!Pawn) return;
UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
if (!ASC) return;
if (GetLocalRole() == ROLE_Authority && !ASC->IsPendingKill())
{
    FScopedPredictionWindow PredictionWindow(ASC, true);
    ASC->RemoveGameplayCue(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x270));
}
if (Pawn->IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
{
    NativeChargeState900Bit6 = true;
    bIsChargingWeapon = false;
    // COMISS zero,ChargeTime + JC takes both greater-than and unordered.
    if (ChargeTime > 0.0f || FMath::IsNaN(ChargeTime) || static_cast<uint8>(NativeLocalChargeStatusPack906) != 0)
    {
        const FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
        NativeSetChargeState1426b21a0(Stats && Stats->ChargeDownTime > 0.0f ? 4 : 0);
    }
}
