// Original address: 0x1420f5ff0
// CONFIDENCE: high
if (MovementMode == MOVE_Custom && CustomMovementMode == 2 && UpdatedComponent && CharacterOwner && CharacterOwner->Controller)
{
    const FRotator CurrentRot = UpdatedComponent->GetComponentRotation();
    const FRotator ControlRot = CharacterOwner->GetControlRotation();
    FRotator TargetRot(0.0f, ControlRot.Yaw, 0.0f);
    if (Velocity.SizeSquared() > 1.0f)
    {
        TargetRot = Velocity.Rotation();
    }
    const FQuat CurrentQuat = CurrentRot.Quaternion();
    const FQuat TargetQuat = TargetRot.Quaternion();
    if (!CurrentQuat.Equals(TargetQuat, 0.0001f))
    {
        MoveUpdatedComponent(FVector::ZeroVector, TargetQuat, true);
    }
}
