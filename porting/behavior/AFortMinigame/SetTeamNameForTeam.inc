// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x1420a5cb0; reflected wrapper 0x1429c6460.
for (FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex)
    {
        Team.TeamName = TeamName;
        return;
    }
}
