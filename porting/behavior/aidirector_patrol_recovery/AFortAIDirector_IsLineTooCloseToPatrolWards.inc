// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Native 0x1416638f0 iterates PatrolWards backwards, prunes null or non-AActor elements,
// invokes GetAffectingDistance via ProcessEvent (0x142a17460), computes the closest point
// on the line segment using FMath::ClosestPointOnSegment (0x142c4b0a0), and tests if the
// ward location is within AffectingDistance of the segment.
static const FName FuncName_GetAffectingDistance(TEXT("GetAffectingDistance"));

for (int32 Index = PatrolWards.Num() - 1; Index >= 0; --Index)
{
    UObject* WardObject = PatrolWards[Index].GetObject();
    if (!WardObject || !WardObject->IsA<AActor>())
    {
        const_cast<AFortAIDirector*>(this)->PatrolWards.RemoveAt(Index, 1, false);
        continue;
    }

    AActor* WardActor = static_cast<AActor*>(WardObject);
    const FVector WardLocation = WardActor->GetActorLocation();

    float AffectingDistance = 0.0f;
    if (UFunction* Func = WardObject->FindFunction(FuncName_GetAffectingDistance))
    {
        WardObject->ProcessEvent(Func, &AffectingDistance);
    }

    const FVector ClosestPoint = FMath::ClosestPointOnSegment(WardLocation, LineStart, LineEnd);
    if (FVector::DistSquared(WardLocation, ClosestPoint) <= AffectingDistance * AffectingDistance)
    {
        return true;
    }
}

return false;
