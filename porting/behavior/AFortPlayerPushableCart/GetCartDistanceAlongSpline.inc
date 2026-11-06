// CONFIDENCE: high
UFortMovementComp_GroundSpline* Component = MovementComponent;
if (Component == nullptr)
{
    return 0.0f;
}
return Component->GetSplineDistance();
