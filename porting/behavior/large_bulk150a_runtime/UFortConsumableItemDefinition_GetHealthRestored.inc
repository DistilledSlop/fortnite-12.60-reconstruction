// CONFIDENCE: high
// 0x14200c400; class resolution 0x1415e2c30, tracked by 0x141de9e00.
float HealthRestored = 0.0f;
for (const FConsumeEffectData& ConsumeEffect : ConsumeEffects)
{
    if (ConsumeEffect.EffectClass.IsNull())
        continue;

    UClass* EffectClass = ConsumeEffect.EffectClass.Get();
    if (!EffectClass)
        EffectClass = Cast<UClass>(FortLoadDelayedObject(ConsumeEffect.EffectClass.ToSoftObjectPath(), true));
    if (EffectClass && !EffectClass->IsChildOf(UGameplayEffect::StaticClass()))
        EffectClass = nullptr;
    FFortAssets::Get().TrackAsset(EffectClass);
    if (!EffectClass)
        continue;

    const UGameplayEffect* Effect = EffectClass->GetDefaultObject<UGameplayEffect>();
    if (Effect && Effect->DurationPolicy == EGameplayEffectDurationType::Instant)
    {
        const FString Context = FString::Printf(TEXT("UFortConsumableItemDefinition::GetHealthRestored from effect %s"), *Effect->GetName());
        for (const FGameplayEffectExecutionDefinition& Execution : Effect->Executions)
        {
            if (Execution.CalculationClass->IsChildOf(UFortHealing::StaticClass()))
                HealthRestored += ConsumeEffect.Level.GetValueAtLevel(static_cast<float>(ItemLevel), &Context);
        }
    }
}
return HealthRestored;
