// Original body address: 0x141ea2470 (thunk 0x14292bc40); leaf loop.
// Offset evidence: this+0xea0 = GameplayVotesArray (reflected TArray<FVoteData>), Num at +0xea8, element stride 0x78; FVoteData.VoteType at element +0. The asm returns true on the first VoteType match with no status test.
// CONFIDENCE: HIGH
for (const FVoteData& Vote : GameplayVotesArray)
{
    if (Vote.VoteType == VoteType)
    {
        return true;
    }
}
return false;
