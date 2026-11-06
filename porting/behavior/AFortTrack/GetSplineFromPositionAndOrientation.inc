// CONFIDENCE: high
// Native 0x1425c3b80 samples nine evenly spaced points on each spline.
USplineComponent* BestSpline = nullptr;
float BestDistance = 0.0f;
float BestDelta = TNumericLimits<float>::Max();
for (USplineComponent* Spline : {SplineComp1, SplineComp2})
{
    if (Spline && Spline->GetNumberOfSplinePoints() > 0)
    {
        const float Length = Spline->GetSplineLength();
        for (int32 Index = 0; Index < 9; ++Index)
        {
            const float Distance = static_cast<float>(Index) * Length * 0.125f;
            const float Delta = FVector::Dist(InLocation, Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World));
            if (Delta < BestDelta)
            {
                BestDelta = Delta;
                BestDistance = Distance;
                BestSpline = Spline;
            }
        }
    }
}
bool bReverse = false;
if (BestSpline)
{
    const FVector SplineDirection = BestSpline->GetDirectionAtDistanceAlongSpline(BestDistance, ESplineCoordinateSpace::World);
    const FVector InputDirection = InRotation.Vector();
    FVector2D SplineXY(SplineDirection.X, SplineDirection.Y);
    FVector2D InputXY(InputDirection.X, InputDirection.Y);
    if (SplineXY.SizeSquared() > SMALL_NUMBER) SplineXY.Normalize();
    if (InputXY.SizeSquared() > SMALL_NUMBER) InputXY.Normalize();
    bReverse = FVector2D::DotProduct(SplineXY, InputXY) < 0.0f;
}
OutDistanceAlongSpline = BestDistance;
OutSpline = BestSpline;
OutReverseDirection = bReverse;
