// CONFIDENCE: high; native wrapper and body captured.
{
    if (SplineCheckPointIndices.IsValidIndex(CheckPointIndex) && SplineComponent.IsValid())
        return SplineComponent.Get()->GetDistanceAlongSplineAtSplinePoint(SplineCheckPointIndices[CheckPointIndex]);
    return -1.0f;
}
