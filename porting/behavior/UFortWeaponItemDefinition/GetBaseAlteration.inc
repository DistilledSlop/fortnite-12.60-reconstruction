// CONFIDENCE: high; native resolver and reflected field mapping recovered.
// The native object resolver returns before tracking when the path is empty.
if (BaseAlteration.IsNull())
    return nullptr;
UFortAlterationItemDefinition* Result = BaseAlteration.Get();
if (!Result)
    Result = Cast<UFortAlterationItemDefinition>(FortLoadDelayedObject(BaseAlteration.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
