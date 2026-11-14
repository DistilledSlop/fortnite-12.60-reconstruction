// Original address: 0x1420ddab0
// CONFIDENCE: high
if (MovementMode != MOVE_Custom || CustomMovementMode != 9)
{
    return false;
}
const float Alt = Params.CustomGravityCeiling.GetValueAtLevel(Level);
if (Alt > 0.0f)
{
    const float HalfWidth = FMath::Max(FMath::Abs(Params.CustomGravityCeilingWidth.GetValueAtLevel(Level)), 100.0f);
    OutMinZ = Alt - HalfWidth;
    OutCenterZ = Alt;
    OutMaxZ = Alt + HalfWidth;
    return true;
}
return false;
