// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d81490 -> 0x141d67f40; 0x141df61e0/0x141de9e00 track the checked class.
UClass* RawClass = nullptr;
if (!PetPrefabClass.IsNull())
{
    RawClass = PetPrefabClass.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(PetPrefabClass.ToSoftObjectPath(), true));
}
TSubclassOf<AFortPlayerPet> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
