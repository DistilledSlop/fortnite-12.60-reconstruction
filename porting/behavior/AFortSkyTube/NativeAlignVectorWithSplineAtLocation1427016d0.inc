// Original address: 0x1427016d0
// CONFIDENCE: high
if (!Spline)
{
    return FVector::ZeroVector;
}
const float InputKey = Spline->FindInputKeyClosestToWorldLocation(WorldLocation);
const FVector Dir = Spline->GetDirectionAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).GetSafeNormal();
return NativeAlignVectorToAxis142701c90(Dir, InputVector);
