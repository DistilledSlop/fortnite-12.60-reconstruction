// CONFIDENCE: high; native loops and reflected team records, see native-evidence.json.
// Native 0x142096c20; reflected wrapper 0x1429c2220.
if (bStableTeamCosmetics)
{
    for (const FMinigamePlayerBucket& Bucket : PlayerBuckets)
    {
        if (Bucket.TeamIdAtRoundStart == TeamIndex) return Bucket.TeamIdAtGameStart;
    }
}
return TeamIndex;
