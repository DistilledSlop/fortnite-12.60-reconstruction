// Original body address: 0x141ea24b0 (thunk 0x14292bcd0); leaf loop.
// Offset evidence: this+0xea0 = GameplayVotesArray (reflected TArray<FVoteData>), Num at +0xea8, element stride 0x78; FVoteData.VoteType at element +0, FVoteData.VoteStatus at element +0x70 (byte compare against 2, JC returns false).
// CONFIDENCE: HIGH
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (Vote.VoteType == VoteType)
    {
        return static_cast<uint8>(Vote.VoteStatus) >= 2;
    }
}
return false;
