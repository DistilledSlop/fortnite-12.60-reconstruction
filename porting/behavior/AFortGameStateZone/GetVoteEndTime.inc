// CONFIDENCE: high
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (Vote.VoteType == VoteType)
    {
        if (static_cast<uint8>(Vote.VoteStatus) > 1)
        {
            VoteEndTime = Vote.VoteEndTime;
            return true;
        }
        break;
    }
}
UE_LOG(LogFort, Warning, TEXT("A vote for %s is not in progress or hasn't ended yet"), *StaticEnum<EFortVoteType>()->GetNameStringByValue(static_cast<int64>(VoteType)));
return false;
