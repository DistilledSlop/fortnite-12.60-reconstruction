// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The wrapper retains a loaded UClass even if it fails the subclass check.
UClass* RawClass = nullptr;
if (!WeaponActorClass.IsNull())
{
    RawClass = WeaponActorClass.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(WeaponActorClass.ToSoftObjectPath(), true));
}
TSubclassOf<AFortWeapon> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
