// Original address: 0x1419adfe0.
// CONFIDENCE: MEDIUM
// Evidence: porting/behavior/aircraft_unlock_runtime/README.md
if (bAircraftIsLocked && !bLocked)
{
    AircraftUnlockPlayerCount = static_cast<int32>(static_cast<uint32>(PlayersLeft) + static_cast<uint32>(PlayerBotsLeft));
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(It->Get());
        if (Controller)
        {
            AFortPlayerStateAthena* State = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
            if (State)
            {
                AFortTeamInfo* Team = State->GetPlayerTeam();
                if (Team)
                {
                    State->AircraftUnlockTeamCount = Team->GetNativeTeamMemberCount();
                }
            }
        }
    }
    if (AFortBroadcastSpectatorInfo* SpectatorInfo = BroadcastSpectatorInfo)
    {
        SpectatorInfo->TotalNumPlayers = PlayersLeft;
        SpectatorInfo->TotalNumTeams = TeamsLeft;
    }
}
bAircraftIsLocked = bLocked;
