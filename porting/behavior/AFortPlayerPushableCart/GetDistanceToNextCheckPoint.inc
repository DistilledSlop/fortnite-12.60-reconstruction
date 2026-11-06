// CONFIDENCE: high; native wrapper and body captured.
{
    if (SplineComponent.IsValid() && MovementComponent != nullptr && CurrentCheckPoint < SplineCheckPointIndices.Num())
    {
        const int32 Next = FMath::Min(static_cast<int32>(CurrentCheckPoint) + 1, SplineCheckPointIndices.Num() - 1);
        const int32 SplineIndex = SplineCheckPointIndices[Next];
        if (SplineIndex >= 0)
        {
            const float Distance = SplineComponent.Get()->GetDistanceAlongSplineAtSplinePoint(SplineIndex) - static_cast<UFortMovementComp_GroundSpline*>(MovementComponent)->GetSplineDistance();
            return Distance <= 0.0f ? 0.0f : Distance;
        }
    }
    return 0.0f;
}
