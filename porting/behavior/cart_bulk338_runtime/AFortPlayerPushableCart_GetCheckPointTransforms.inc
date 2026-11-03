// CONFIDENCE: high; native wrapper, body and spline point helper captured.
{
    if (SplineComponent.IsValid())
    {
        OutTransforms.SetNum(SplineCheckPointIndices.Num());
        for (int32 Index = 0; Index < SplineCheckPointIndices.Num(); ++Index)
            OutTransforms[Index] = SplineComponent.Get()->GetTransformAtSplinePoint(SplineCheckPointIndices[Index], ESplineCoordinateSpace::World, false);
    }
}
