// CONFIDENCE: high
// Native target 0x142012d30.
if (UseAnimation.IsNull())
    return nullptr;
UAnimMontage* Result = UseAnimation.Get();
if (!Result)
    Result = Cast<UAnimMontage>(FortLoadDelayedObject(UseAnimation.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
