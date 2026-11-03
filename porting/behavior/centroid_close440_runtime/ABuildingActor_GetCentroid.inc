// CONFIDENCE: high; native rotates CentroidOffset without scale, then adds root translation.
{
    const FQuat Rotation = RootComponent ? RootComponent->GetComponentTransform().GetRotation() : FQuat::Identity;
    const FVector Location = RootComponent ? RootComponent->GetComponentTransform().GetTranslation() : FVector::ZeroVector;
    const FVector Offset = CentroidOffset;
    float TX = Offset.Z * Rotation.Y - Offset.Y * Rotation.Z;
    float TY = Offset.X * Rotation.Z - Offset.Z * Rotation.X;
    float TZ = Offset.Y * Rotation.X - Offset.X * Rotation.Y;
    TX = TX + TX;
    TY = TY + TY;
    TZ = TZ + TZ;
    const float XBase = TX * Rotation.W + Offset.X;
    const float YBase = TY * Rotation.W + Offset.Y;
    const float ZBase = TZ * Rotation.W + Offset.Z;
    return FVector(
        ((TZ * Rotation.Y - TY * Rotation.Z) + XBase) + Location.X,
        ((TX * Rotation.Z - TZ * Rotation.X) + YBase) + Location.Y,
        ((TY * Rotation.X - TX * Rotation.Y) + ZBase) + Location.Z);
}
