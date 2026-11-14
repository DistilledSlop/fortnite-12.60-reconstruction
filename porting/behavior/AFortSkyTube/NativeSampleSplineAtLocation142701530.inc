// Original address: 0x142701530
// CONFIDENCE: high
if (!Spline)
{
    OutInputKey = 0.0f;
    OutLocation = FVector::ZeroVector;
    OutDirection = FVector::ZeroVector;
    return;
}
OutInputKey = Spline->FindInputKeyClosestToWorldLocation(WorldLocation);
OutLocation = Spline->GetLocationAtSplineInputKey(OutInputKey, ESplineCoordinateSpace::World);
OutDirection = Spline->GetDirectionAtSplineInputKey(OutInputKey, ESplineCoordinateSpace::World).GetSafeNormal();
