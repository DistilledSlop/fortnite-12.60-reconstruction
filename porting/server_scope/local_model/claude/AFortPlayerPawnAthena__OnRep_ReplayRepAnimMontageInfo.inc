// CONFIDENCE: HIGH
// Original: exec thunk 0x142a83890. Callees: CopyReplicatedMontageInfoToASC = native 0x1406daaf0 (dep), GetFortAbilitySystemComponentClass = static class getter 0x14277e480 (dep),
// OnRep_ReplicatedAnimMontage = virtual slot 0x7d8 on the ability system component.
// No reflected parameters. The replay copy is a full struct assign (0x38 bytes).
RepAnimMontageInfo = ReplayRepAnimMontageInfo;

UAbilitySystemComponent* Asc = AbilitySystemComponent;
if (Asc != nullptr && Asc->IsA(GetFortAbilitySystemComponentClass()))
{
    // Re-apply the replicated montage state on the ability system component (native 0x1406daaf0 copies the 0x38-byte info).
    CopyReplicatedMontageInfoToASC(Asc, RepAnimMontageInfo);

    // Copy the landing montage pair into the ASC (ASC+0x14a8 .. +0x14c8).
    Asc->LandingMontagePairPrivate = LandingMontagePair;

    // Notify the ASC that the replicated montage changed (virtual slot 0x7d8).
    Asc->OnRep_ReplicatedAnimMontage();
}
