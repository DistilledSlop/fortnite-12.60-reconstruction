// CONFIDENCE: HIGH (source recovery; build/runtime pending)
// Native 0x141dd4d50. Engine TQuadTree<int32,16> matches recovered spatial implementation.
// Signature: void AFortWindManager::NativeRegisterWindResponder(FFortWindResponder& Responder)
ABuildingSMActor* const Actor = Responder.WindUpdatingBuildingSMActor;
if (!Responder.WindSpeedCurve || !Responder.WindPannerSpeedCurve)
{
    if (UE_LOG_ACTIVE(LogFortWind, Error))
    {
        const UClass* const ActorClass = Actor->GetClass();
        const UObject* const Defaults = ActorClass ? ActorClass->GetDefaultObject() : nullptr;
        const FString BlueprintPath = Defaults ? Defaults->GetPathName() : TEXT("<None>");
        if (!Responder.WindSpeedCurve)
        {
            UE_LOG(LogFortWind, Error, TEXT("WindResponder (%s, path: %s, blueprint: %s) failed to register for wind.  Missing WindSpeedCurve!"),
                *Actor->GetName(), *Actor->GetPathName(), *BlueprintPath);
        }
        else
        {
            UE_LOG(LogFortWind, Error, TEXT("WindResponder (%s, path: %s, blueprint: %s) failed to register for wind.  Missing WindPannerSpeedCurve!"),
                *Actor->GetName(), *Actor->GetPathName(), *BlueprintPath);
        }
    }
    return;
}

const FBox Bounds = Actor->GetComponentsBoundingBox(false, false);
static_assert(sizeof(Bounds) == sizeof(Responder.NativeBounds_0x68), "native wind bounds layout changed");
FMemory::Memcpy(Responder.NativeBounds_0x68, &Bounds, sizeof(Bounds));
TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
Actor->GetComponents(PrimitiveComponents, false);
bool bHasLODParent = false;
for (const UPrimitiveComponent* Component : PrimitiveComponents)
{
    if (Component->GetLODParentPrimitive())
    {
        bHasLODParent = true;
        break;
    }
}
Responder.NativeEffectiveDistance_0x88 = bHasLODParent
    ? UpdateWindMaxResponderDistance
    : (UpdateWindMaxResponderDistance <= Actor->CullDistance ? UpdateWindMaxResponderDistance : Actor->CullDistance);

// No duplicate search: native copies and appends even when this actor is already registered.
const int32 Index = WindResponders.Add(Responder);
if (!NativeWindResponderTree_0x2e0)
{
    const float Width = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.WindResponderQuadTreeSize"))->GetFloat();
    const float MinimumSize = static_cast<float>(static_cast<int32>(IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.WindResponderQuadTreeCellSize"))->GetFloat()));
    const float HalfWidth = Width * 0.5f;
    NativeWindResponderTree_0x2e0 = MakeUnique<TQuadTree<int32, 16>>(
        FBox2D(FVector2D(Width * -0.5f, Width * -0.5f), FVector2D(HalfWidth, HalfWidth)), MinimumSize);
}
NativeWindResponderTree_0x2e0->Insert(Index, FBox2D(FVector2D(Bounds.Min.X, Bounds.Min.Y), FVector2D(Bounds.Max.X, Bounds.Max.Y)), nullptr);

const FVector Center = (Bounds.Min + Bounds.Max) * 0.5f;
const FVector Delta = Center - NativeNearbyCachePosition_0x30c;
const float DistanceSquared = Delta.X * Delta.X + Delta.Y * Delta.Y + Delta.Z * Delta.Z;
const float ScaledDistance = GetCachedScalabilityCVars().ViewDistanceScale * Responder.NativeEffectiveDistance_0x88;
const float UpdateRadius = (UpdateWindMaxResponderDistance <= ScaledDistance ? UpdateWindMaxResponderDistance : ScaledDistance)
    + ViewerMovementDistanceForRefresh;
if (DistanceSquared < UpdateRadius * UpdateRadius)
    NearbyCacheIndicesForUpdates.Add(Index);
if (bAllowResponderAudio)
{
    const float AudioRadius = AudioWindMaxResponderDistance + ViewerMovementDistanceForRefresh;
    if (DistanceSquared < AudioRadius * AudioRadius)
        NearbyCacheIndicesForAudio.Add(Index);
}
