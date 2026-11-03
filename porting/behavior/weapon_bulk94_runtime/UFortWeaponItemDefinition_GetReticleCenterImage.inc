// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (ReticleCenterImage.IsNull())
    return nullptr;
UTexture2D* Result = ReticleCenterImage.Get();
if (!Result)
    Result = Cast<UTexture2D>(FortLoadDelayedObject(ReticleCenterImage.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
