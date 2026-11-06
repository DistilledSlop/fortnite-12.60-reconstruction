// CONFIDENCE: high
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (Vote.VoteType == VoteType)
    {
        ArbitratorType = Vote.VoteConfig.VoteArbitratorType;
        return true;
    }
}
UE_LOG(LogFort, Warning, TEXT("A vote for %s is not in progress"), *StaticEnum<EFortVoteType>()->GetNameStringByValue(static_cast<int64>(VoteType)));
ArbitratorType = static_cast<EFortVoteArbitratorType>(0);
return false;
