// CONFIDENCE: high; native capture and reflected fields reviewed.
uint32 Total = 0;
if (bHasRewards)
{
	for (const FAthenaAwardGroup& Reward : EndOfMatchResults.Rewards)
	{
		Total += static_cast<uint32>(Reward.Score);
	}
}
return static_cast<int32>(Total);
