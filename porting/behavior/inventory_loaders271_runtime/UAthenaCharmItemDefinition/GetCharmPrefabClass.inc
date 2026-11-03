// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d47c50 -> 0x141d27940; 0x141df61e0/0x141de9e00 track the checked class.
UClass* RawClass = nullptr;
if (!CharmPrefabClass.IsNull())
{
    RawClass = CharmPrefabClass.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(CharmPrefabClass.ToSoftObjectPath(), true));
}
TSubclassOf<AFortPlayerCharm> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
