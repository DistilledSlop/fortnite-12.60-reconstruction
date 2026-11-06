// CONFIDENCE: high
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (Vote.VoteType == VoteType)
    {
        return Vote.VoteConfig.VoteDuration;
    }
}
UE_LOG(LogFort, Warning, TEXT("A vote for %s is not in progress"), *StaticEnum<EFortVoteType>()->GetNameStringByValue(static_cast<int64>(VoteType)));
return -1.0f;
