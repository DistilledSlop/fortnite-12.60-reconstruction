// CONFIDENCE: high; see native-evidence.json for captured native control flow.
// Native 0x142096b50; reflected wrapper 0x1429c2000.
const FFortMinigameStatCollection& Collection = bGameTotal ? GameStats : RoundStats;
for (const FFortMinigamePlayerStats& Stats : Collection.PlayerStats)
{
    if (Stats.Player == PlayerId)
    {
        for (const FFortMinigameStat& Stat : Stats.Stats)
        {
            if (Stat.Filter.Get() == InFilter.Get()) return Stat.Count;
        }
        return 0;
    }
}
return 0;
