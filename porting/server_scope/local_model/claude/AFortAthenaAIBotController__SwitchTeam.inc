// CONFIDENCE: MEDIUM. Original exec 0x1427c36e0; native body 0x1416b0620 (inlined). Parameter: TeamIndex (uint8).
// Only team indices above 1 act. Team 2 is set directly on the PlayerState. Other teams are placed on the team
// through the game-state team lookup (vtable slot 0xb10). The PerceptionComponent is notified last.
if (TeamIndex > 1 && PlayerState && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
{
    if (TeamIndex == 2)
    {
        PlayerState->NativeRecoveredByte_0xe60 = 2;
    }
    else
    {
        UWorld* World = GetWorld();
        UObject* GameStateLike = World ? GetWorldGameStateForTeams(World) : nullptr;
        UObject* TeamInfo = GameStateLike ? GameStateLike->NativeGetTeamInfoForIndex(TeamIndex) : nullptr;
        if (TeamInfo && TeamInfo->IsA(AFortTeamInfoAthena::StaticClass()))
        {
            PlaceBotOnTeam(GameStateLike, PlayerState, TeamInfo, PlayerState->NativeRecoveredUInt8_0xfdc);
        }
    }

    if (PerceptionComponent)
    {
        NotifyBotTeamChanged();
    }
}
