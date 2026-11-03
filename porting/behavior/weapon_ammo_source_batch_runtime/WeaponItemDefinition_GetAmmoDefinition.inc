// CONFIDENCE: HIGH
// 0x142669040 -> 0x1424ce740, timing enabled.
if (AmmoData.IsNull())
    return nullptr;
UFortWorldItemDefinition* Result = AmmoData.Get();
if (!Result)
    Result = Cast<UFortWorldItemDefinition>(FortLoadDelayedObject(AmmoData.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
