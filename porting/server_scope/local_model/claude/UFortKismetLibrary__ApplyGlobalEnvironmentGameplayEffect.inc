// CONFIDENCE: MEDIUM. Applies GameplayEffect to the game state's (global environment) ability system component, using the effect's class default object. The native apply path (0x141ee57e0 -> 0x14161dc00) reads GameplayEffectLevel and AdditionalContextTags from the frame but does not forward them, so they are not used here.
// Original address: 0x142966930 (exec). Native helpers: 0x141ee57e0 (dispatch), 0x141ef3af0 (game state ASC), 0x14161dc00 (apply to self).
if (WorldContextObject == nullptr || GameplayEffect.Get() == nullptr || !GameplayEffect.Get()->IsChildOf(UGameplayEffect::StaticClass()))
{
    return;
}
UAbilitySystemComponent* GlobalASC = GetGameStateAbilitySystemComponent(WorldContextObject);
if (GlobalASC == nullptr)
{
    return;
}
GlobalASC->ApplyGameplayEffectToSelf(GameplayEffect.GetDefaultObject(), 1.0f, GlobalASC->MakeEffectContext());
