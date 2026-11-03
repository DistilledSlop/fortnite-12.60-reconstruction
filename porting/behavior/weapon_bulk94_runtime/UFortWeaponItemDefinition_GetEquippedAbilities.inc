// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// Build a fresh return array in source order; track every element, including null.
TArray<TSubclassOf<UFortGameplayAbility>> Result;
for (const TSoftClassPtr<UFortGameplayAbility>& Ability : EquippedAbilities)
{
    UClass* RawClass = nullptr;
    if (!Ability.IsNull())
    {
        RawClass = Ability.Get();
        if (!RawClass)
            RawClass = Cast<UClass>(FortLoadDelayedObject(Ability.ToSoftObjectPath(), true));
    }
    TSubclassOf<UFortGameplayAbility> CheckedClass(RawClass);
    FFortAssets::Get().TrackAsset(CheckedClass.Get());
    if (UClass* ValidClass = CheckedClass.Get())
        Result.Add(ValidClass);
}
return Result;
