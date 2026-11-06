// CONFIDENCE: high; native wrapper and body captured.
{
    if (SplineComponent.IsValid() && MovementComponent != nullptr && CurrentCheckPoint < SplineCheckPointIndices.Num())
    {
        const int32 CurrentIndex = SplineCheckPointIndices[CurrentCheckPoint];
        const int32 NextIndex = SplineCheckPointIndices[FMath::Min(static_cast<int32>(CurrentCheckPoint) + 1, SplineCheckPointIndices.Num() - 1)];
        if (CurrentIndex >= 0 && NextIndex >= 0)
        {
            const float CurrentDistance = SplineComponent.Get()->GetDistanceAlongSplineAtSplinePoint(CurrentIndex);
            const float NextDistance = SplineComponent.Get()->GetDistanceAlongSplineAtSplinePoint(NextIndex);
            const float Progress = (static_cast<UFortMovementComp_GroundSpline*>(MovementComponent)->GetSplineDistance() - CurrentDistance) / (NextDistance - CurrentDistance);
            return Progress <= 0.0f ? 0.0f : Progress;
        }
    }
    return 0.0f;
}
