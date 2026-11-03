// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x1420a3f60; reflected wrapper 0x1429c5b90.
for (FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex)
    {
        if (Team.MaxInitTeamSize < 0) Team.MaxInitTeamSize = TeamSize;
        return;
    }
}
