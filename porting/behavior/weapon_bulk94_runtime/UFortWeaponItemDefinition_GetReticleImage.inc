// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (ReticleImage.IsNull())
    return nullptr;
UTexture2D* Result = ReticleImage.Get();
if (!Result)
    Result = Cast<UTexture2D>(FortLoadDelayedObject(ReticleImage.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
