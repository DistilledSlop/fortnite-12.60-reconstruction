// CONFIDENCE: HIGH
FFortWindImpulseCylinder Result;
Result.Location = ImpulseLocation;
Result.InnerRadius = ImpulseInnerRadius;
Result.OuterRadius = ImpulseOuterRadius;
Result.Magnitude = ImpulseMagnitude;
Result.WorldBounds = FBox(
    FVector(ImpulseLocation.X - ImpulseOuterRadius, ImpulseLocation.Y - ImpulseOuterRadius, ImpulseLocation.Z),
    FVector(ImpulseLocation.X + ImpulseOuterRadius, ImpulseLocation.Y + ImpulseOuterRadius, ImpulseLocation.Z + 100000.0f));
Result.bIsChanging = false;
Result.bIsChangePending = false;
Result.WindDirection = ImpulseDirection;
return Result;
