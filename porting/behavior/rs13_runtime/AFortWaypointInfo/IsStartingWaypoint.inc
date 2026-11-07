// Original address: 0x141f8f520 (thunk 0x142b474c0). Offset evidence: Waypoints is TArray<ABuildingTrapFloor_Waypoint*> at +0x218 (reflected AFortWaypointInfo.Waypoints, Num at +0x220); the body finds the first non-null element and compares it with the argument.
// CONFIDENCE: HIGH
if (Waypoint == nullptr)
{
    return false;
}
if (Waypoints.Num() <= 0)
{
    return false;
}
for (int32 Index = 0; Index < Waypoints.Num(); ++Index)
{
    if (Waypoints[Index] != nullptr)
    {
        return Waypoints[Index] == Waypoint;
    }
}
return false;
