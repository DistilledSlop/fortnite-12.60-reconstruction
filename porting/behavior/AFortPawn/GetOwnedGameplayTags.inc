// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original 0x1423233c0, interface this = pawn + 0x4b0.
TagContainer.Reset();
if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
{
    ASC->GetOwnedGameplayTags(TagContainer);
}
