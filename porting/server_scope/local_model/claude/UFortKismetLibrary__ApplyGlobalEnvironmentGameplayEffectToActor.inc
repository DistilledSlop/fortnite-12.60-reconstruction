// CONFIDENCE: MEDIUM. Applies GameplayEffect from the global environment ability system component (found through EffectTargetActor's world) to EffectTargetActor's own ability system component. The spec gets GameplayEffectLevel and AdditionalContextTags (tags are appended to its dynamic asset tags). Returns the active handle, or an invalid handle when any lookup or spec step fails.
// Original address: 0x142966ab0 (exec). Native helpers: 0x141ee5840 (dispatch), 0x141ef3af0 (game state ASC), 0x142ec9900 (target ASC), 0x14161dd60 (apply to target), 0x14162cef0 (spec with level and tags).
FActiveGameplayEffectHandle Result; // invalid handle by default, as in the native (-1) handle
if (EffectTargetActor == nullptr || GameplayEffect.Get() == nullptr || !GameplayEffect.Get()->IsChildOf(UGameplayEffect::StaticClass()))
{
    return Result;
}
UAbilitySystemComponent* GlobalASC = GetGameStateAbilitySystemComponent(EffectTargetActor);
UAbilitySystemComponent* TargetASC = EffectTargetActor->FindComponentByClass<UAbilitySystemComponent>();
if (GlobalASC == nullptr || TargetASC == nullptr)
{
    return Result;
}
FGameplayEffectSpecHandle SpecHandle = GlobalASC->MakeOutgoingSpec(GameplayEffect, GameplayEffectLevel, GlobalASC->MakeEffectContext());
if (!SpecHandle.IsValid())
{
    return Result;
}
SpecHandle.Data->AppendDynamicAssetTags(AdditionalContextTags);
Result = GlobalASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
