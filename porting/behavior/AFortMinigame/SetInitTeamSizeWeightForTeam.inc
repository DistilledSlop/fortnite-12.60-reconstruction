// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x1420a3df0; reflected wrapper 0x1429c5ad0.
for (FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex)
    {
        Team.InitTeamSizeWeight = TeamSizeWeight;
        return;
    }
}
