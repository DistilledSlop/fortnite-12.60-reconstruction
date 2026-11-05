// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Native 0x141663ad0 iterates PatrolWards backwards, prunes null or non-AActor elements,
// queries GetAffectingType via ProcessEvent (0x142a174b0) to filter by EWardAffectType,
// queries GetAffectingDistance via ProcessEvent (0x142a17460), and tests whether Point
// is within AffectingDistance of the ward location.
static const FName FuncName_GetAffectingType(TEXT("GetAffectingType"));
static const FName FuncName_GetAffectingDistance(TEXT("GetAffectingDistance"));

for (int32 Index = PatrolWards.Num() - 1; Index >= 0; --Index)
{
    UObject* WardObject = PatrolWards[Index].GetObject();
    if (!WardObject || !WardObject->IsA<AActor>())
    {
        const_cast<AFortAIDirector*>(this)->PatrolWards.RemoveAt(Index, 1, false);
        continue;
    }

    uint8 AffectTypeByte = 0;
    if (UFunction* FuncType = WardObject->FindFunction(FuncName_GetAffectingType))
    {
        WardObject->ProcessEvent(FuncType, &AffectTypeByte);
    }
    const EWardAffectType AffectType = static_cast<EWardAffectType>(AffectTypeByte);

    if (WardEffectTypeFilter != EWardAffectType::AffectsBothStartAndEndPoints &&
        AffectType != EWardAffectType::AffectsBothStartAndEndPoints &&
        WardEffectTypeFilter != AffectType)
    {
        continue;
    }

    AActor* WardActor = static_cast<AActor*>(WardObject);
    const FVector WardLocation = WardActor->GetActorLocation();

    float AffectingDistance = 0.0f;
    if (UFunction* FuncDist = WardObject->FindFunction(FuncName_GetAffectingDistance))
    {
        WardObject->ProcessEvent(FuncDist, &AffectingDistance);
    }

    if (FVector::DistSquared(Point, WardLocation) <= AffectingDistance * AffectingDistance)
    {
        return true;
    }
}

return false;
