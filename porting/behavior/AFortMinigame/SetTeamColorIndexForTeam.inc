// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x1420a57e0; reflected wrapper 0x1429c6240.
for (FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == TeamIndex)
    {
        Team.TeamColorIndex = TeamColorIndex;
        return;
    }
}
