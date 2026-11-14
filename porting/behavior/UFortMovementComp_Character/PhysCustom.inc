// Original address: 0x1420e85a0
// CONFIDENCE: high
if (MovementMode == MOVE_Custom)
{
    if (CustomMovementMode == 5)
    {
        NativePhysCustomMode5_1420e9390(deltaTime, Iterations);
        return;
    }
    if (CustomMovementMode == 3 || CustomMovementMode == 4 ||
        CustomMovementMode == 9 || CustomMovementMode == 12)
    {
        const uint8 SavedAirFlag = NativeAirborneFlag;
        NativeAirborneFlag = 1;
        CustomFallingAcceleration = ((NativePackedAccelFlags_1e38 & 8) != 0) ? ReplicatedAcceleration : Acceleration;
        NativeAirborneVelocity_1e0c = Velocity;
        PhysFalling(deltaTime, Iterations);
        NativeAirborneFlag = SavedAirFlag;
        return;
    }
    if (CustomMovementMode == 6)
    {
        NativePhysCustomMode6_1420e8700(deltaTime, Iterations);
        return;
    }
    if (CustomMovementMode == 7)
    {
        NativePhysCustomMode7_1420e9010(deltaTime, Iterations);
        return;
    }
    if (CustomMovementMode == 8)
    {
        NativePhysCustomMode8_1420ea420(deltaTime, Iterations);
        return;
    }
    if (CustomMovementMode == 10)
    {
        NativePhysCustomMode10_1420e9ae0(deltaTime, Iterations);
        return;
    }
}
Super::PhysCustom(deltaTime, Iterations);
