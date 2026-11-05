// CONFIDENCE: HIGH
// Native 0x141de5910 constructs into caller-owned storage. The input is required.
FFortWindResponder Responder;
Responder.WindUpdatingBuildingSMActor = Building;
Responder.WindSpeedCurve = nullptr;
Responder.WindPannerSpeedCurve = nullptr;
Responder.WindAudio = nullptr;
Responder.MaterialParameterPairIndices = 0;
Responder.WindSpeed = 0.0f;
Responder.NativeEffectiveDistance_0x88 = 0.0f;
Responder.bHasSetupAnimatingMaterials = false;

// The native resolvers track every nonempty attempt, including a null result.
const auto Resolve = [](UObject* Cached, const FSoftObjectPath& Path, UClass* ExpectedClass) -> UObject*
{
    if (Path.IsNull())
        return nullptr;
    // TSoftObjectPtr::Get already resolves and refreshes its persistent weak cache.
    UObject* Object = Cached;
    if (!Object || !Object->IsA(ExpectedClass))
        Object = FortLoadDelayedObject(Path, true);
    if (Object && !Object->IsA(ExpectedClass))
        Object = nullptr;
    FFortAssets::Get().TrackAsset(Object);
    return Object;
};

const ABuildingSMActor* Defaults = Building->GetClass()
    ? Building->GetClass()->GetDefaultObject<ABuildingSMActor>() : nullptr;
if (Defaults)
{
    Responder.WindSpeedCurve = Cast<UCurveLinearColor>(Resolve(Defaults->WindSpeedCurve.Get(), Defaults->WindSpeedCurve.ToSoftObjectPath(), UCurveLinearColor::StaticClass()));
    Responder.WindPannerSpeedCurve = Cast<UCurveLinearColor>(Resolve(Defaults->WindPannerSpeedCurve.Get(), Defaults->WindPannerSpeedCurve.ToSoftObjectPath(), UCurveLinearColor::StaticClass()));
    Responder.WindAudio = Cast<USoundBase>(Resolve(Defaults->WindAudio.Get(), Defaults->WindAudio.ToSoftObjectPath(), USoundBase::StaticClass()));
}
if (!Responder.WindSpeedCurve)
    Responder.WindSpeedCurve = Cast<UCurveLinearColor>(Resolve(Building->WindSpeedCurve.Get(), Building->WindSpeedCurve.ToSoftObjectPath(), UCurveLinearColor::StaticClass()));
if (!Responder.WindPannerSpeedCurve)
    Responder.WindPannerSpeedCurve = Cast<UCurveLinearColor>(Resolve(Building->WindPannerSpeedCurve.Get(), Building->WindPannerSpeedCurve.ToSoftObjectPath(), UCurveLinearColor::StaticClass()));
if (!Responder.WindAudio)
    Responder.WindAudio = Cast<USoundBase>(Resolve(Building->WindAudio.Get(), Building->WindAudio.ToSoftObjectPath(), USoundBase::StaticClass()));

if (Building->StaticMeshComponent)
{
    for (const TSoftObjectPtr<UMaterialInterface>& SoftMaterial : Building->IntenseWindMaterials)
    {
        if (UMaterialInterface* Material = Cast<UMaterialInterface>(Resolve(SoftMaterial.Get(), SoftMaterial.ToSoftObjectPath(), UMaterialInterface::StaticClass())))
            Responder.IntenseWindMaterialInstances.Add(Material);
    }
    if (Responder.IntenseWindMaterialInstances.Num() > 0)
    {
        Responder.MildWindMaterialInstances = Building->StaticMeshComponent->GetMaterials();
        Responder.IntenseWindMaterialInstances.Reserve(Responder.MildWindMaterialInstances.Num());
        for (int32 Index = 0; Index < Responder.MildWindMaterialInstances.Num(); ++Index)
        {
            if (Index >= Responder.IntenseWindMaterialInstances.Num())
                Responder.IntenseWindMaterialInstances.Add(Responder.MildWindMaterialInstances[Index]);
            else if (!Responder.IntenseWindMaterialInstances[Index])
                Responder.IntenseWindMaterialInstances[Index] = Responder.MildWindMaterialInstances[Index];
        }
    }
}
return Responder;
