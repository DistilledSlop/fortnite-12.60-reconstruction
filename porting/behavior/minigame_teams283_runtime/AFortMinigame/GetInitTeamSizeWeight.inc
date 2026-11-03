// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x14208fe00; reflected wrapper 0x1429c0540.
for (const FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex) return Team.InitTeamSizeWeight;
}
if (TeamIndex != 255)
{
    for (const FMinigameTeam& Team : TeamArray)
    {
        if (Team.TeamIndex == 255) return Team.InitTeamSizeWeight;
    }
}
return 1;
