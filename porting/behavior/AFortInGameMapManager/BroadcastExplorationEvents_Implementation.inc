// CONFIDENCE: high. Native target 0x141ecbc10.
for (const FTeamMapExplorationEvent& Event : Events)
{
    if (Event.TeamId == NativeExplorationTeam)
    {
        if (Event.ExplorationThreshold == 127)
        {
            OnMapFullyExplored.Broadcast(Event.TeamId);
        }
        else
        {
            OnMapExplorationThresholdReached.Broadcast(static_cast<int32>(Event.ExplorationThreshold) - 1, Event.TeamId);
        }
    }
}
