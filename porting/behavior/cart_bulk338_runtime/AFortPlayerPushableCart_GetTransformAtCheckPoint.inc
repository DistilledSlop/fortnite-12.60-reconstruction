// CONFIDENCE: high; native wrapper, body and spline point helper captured.
{
    if (SplineComponent.IsValid())
        OutTransform = SplineComponent.Get()->GetTransformAtSplinePoint(SplineCheckPointIndices[CheckPointIndex], ESplineCoordinateSpace::World, false);
}
