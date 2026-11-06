// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (ReticleCenterPerfectAimImage.IsNull())
    return nullptr;
UTexture2D* Result = ReticleCenterPerfectAimImage.Get();
if (!Result)
    Result = Cast<UTexture2D>(FortLoadDelayedObject(ReticleCenterPerfectAimImage.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
