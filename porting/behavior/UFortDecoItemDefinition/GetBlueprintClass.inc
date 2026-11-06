// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x142009ac0 -> 0x141ff85e0; 0x141df61e0/0x141de9e00 track the checked class.
UClass* RawClass = nullptr;
if (!BlueprintClass.IsNull())
{
    RawClass = BlueprintClass.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(BlueprintClass.ToSoftObjectPath(), true));
}
TSubclassOf<ABuildingActor> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
