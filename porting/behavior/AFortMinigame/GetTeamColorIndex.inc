// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x142096ba0; reflected wrapper 0x1429c2190.
const uint8 CosmeticTeamIndex = GetTeamIndexForCosmeticDetails(TeamIndex);
for (const FMinigameTeam& Team : TeamArray)
{
    if (Team.TeamIndex == CosmeticTeamIndex) return Team.TeamColorIndex;
}
if (CosmeticTeamIndex != 255)
{
    for (const FMinigameTeam& Team : TeamArray)
    {
        if (Team.TeamIndex == 255) return Team.TeamColorIndex;
    }
}
return -1;
