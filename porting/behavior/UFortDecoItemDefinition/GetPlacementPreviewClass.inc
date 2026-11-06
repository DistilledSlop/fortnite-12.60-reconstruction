// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x14200fe40 -> 0x141ff87f0; 0x141df61e0/0x141de9e00 track the checked class.
UClass* RawClass = nullptr;
if (!PlacementPreviewClass.IsNull())
{
    RawClass = PlacementPreviewClass.Get();
    if (!RawClass)
        RawClass = Cast<UClass>(FortLoadDelayedObject(PlacementPreviewClass.ToSoftObjectPath(), true));
}
TSubclassOf<AFortDecoPreview> Result(RawClass);
FFortAssets::Get().TrackAsset(Result.Get());
return Result;
