// CONFIDENCE: high
// Native helper 0x141f8f0e0.
if (Waypoint == nullptr)
{
    return false;
}
for (int32 Index = Waypoints.Num() - 1; Index >= 0; --Index)
{
    ABuildingTrapFloor_Waypoint* CurWaypoint = Waypoints[Index];
    if (CurWaypoint != nullptr)
    {
        return CurWaypoint == Waypoint;
    }
}
return false;
