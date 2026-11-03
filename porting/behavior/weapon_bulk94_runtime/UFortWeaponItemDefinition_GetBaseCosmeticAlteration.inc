// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (BaseCosmeticAlteration.IsNull())
    return nullptr;
UFortAlterationItemDefinition* Result = BaseCosmeticAlteration.Get();
if (!Result)
    Result = Cast<UFortAlterationItemDefinition>(FortLoadDelayedObject(BaseCosmeticAlteration.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
