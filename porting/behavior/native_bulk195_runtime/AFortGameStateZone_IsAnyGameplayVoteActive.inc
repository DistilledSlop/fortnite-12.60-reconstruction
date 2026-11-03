// Native body 0x141ea2160. CONFIDENCE: high
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (static_cast<uint8>(Vote.VoteStatus) < 2) return true;
}
return false;
