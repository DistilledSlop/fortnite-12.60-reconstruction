// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (ReticleInvalidTargetImage.IsNull())
    return nullptr;
UTexture2D* Result = ReticleInvalidTargetImage.Get();
if (!Result)
    Result = Cast<UTexture2D>(FortLoadDelayedObject(ReticleInvalidTargetImage.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
