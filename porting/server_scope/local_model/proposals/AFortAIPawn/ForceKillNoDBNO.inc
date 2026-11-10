// CONFIDENCE: medium-high (body fully read; depends on the AFortPawn virtual KillWithDeathTag declared by work/dependencies/pawn-kill-virtuals)
// Native body 0x1427a47c0 (reflected exec, flags per reflection). Original:
//   if (Role == ROLE_Authority && !bIsDying) { bUsesDBNO = false; LastHitBy = nullptr;
//     this->vtable[0xa80](FFortNativeGameplayTags[+0x440] = Gameplay.Effect.InstantDeath.Silent, nullptr, this); }
if (GetLocalRole() == ROLE_Authority && !bIsDying)
{
    bUsesDBNO = false;
    LastHitBy = nullptr;
    KillWithDeathTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x440), nullptr, this);
}
