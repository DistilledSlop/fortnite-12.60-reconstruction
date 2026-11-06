// CONFIDENCE: high; see native-evidence.json for captured native control flow.
// Native 0x142096920; reflected wrapper 0x1429c1d40.
const FFortMinigameGroupStats& Stats = bGameTotal ? GameStats.GroupStats : RoundStats.GroupStats;
for (const FFortMinigameStat& Stat : Stats.Stats)
{
    if (Stat.Filter.Get() == InFilter.Get()) return Stat.Count;
}
return 0;
