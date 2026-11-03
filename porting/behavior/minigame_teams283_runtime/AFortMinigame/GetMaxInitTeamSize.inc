// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x142091e60; reflected wrapper 0x1429c05d0.
for (const FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex) return Team.MaxInitTeamSize;
}
if (TeamIndex != 255)
{
    for (const FMinigameTeam& Team : TeamArray)
    {
        if (Team.TeamIndex == 255) return Team.MaxInitTeamSize;
    }
}
return -1;
