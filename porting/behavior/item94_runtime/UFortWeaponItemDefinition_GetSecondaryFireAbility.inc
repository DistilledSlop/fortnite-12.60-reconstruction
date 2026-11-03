// CONFIDENCE: high; native leaf and shared resolver recovered.
// 0x141680900: keep the raw class in the return wrapper; TrackAsset receives its checked value.
UClass* RawClass = nullptr;
if (!SecondaryFireAbility.IsNull())
{
    RawClass = SecondaryFireAbility.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(SecondaryFireAbility.ToSoftObjectPath(), true));
}
TSubclassOf<UFortGameplayAbility> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
