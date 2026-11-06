// CONFIDENCE: high
// Native helper 0x14181b720.
if (FMath::Abs(Angle) <= DeadZoneAngle + 45.0f)
{
    return EFortCardinalDirection::North;
}
if (135.0f - DeadZoneAngle <= FMath::Abs(Angle))
{
    return EFortCardinalDirection::South;
}
if (Angle > 0.0f)
{
    return EFortCardinalDirection::East;
}
return EFortCardinalDirection::West;
