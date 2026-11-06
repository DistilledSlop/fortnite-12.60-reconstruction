// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x142546d60 -> 0x141d99020; empty paths return before tracking.
if (PackImage.IsNull())
    return nullptr;
UTexture* Result = PackImage.Get();
if (!Result)
    Result = Cast<UTexture>(FortLoadDelayedObject(PackImage.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
