// Original body address: 0x141b22290
// Evidence: range check of RoundNumber against RoundData.SingleRoundInfos (reflected struct at +0x7a0; array Data at +0x7a8, Num at +0x7b0; element stride 0x30, RoundTeamWinner uint8 at element +0). Out-of-range returns 0xff.
// Verbosity-gated diagnostic log at call 0x142c54b70 elided on both paths (no state change).
// CONFIDENCE: MEDIUM
if (RoundNumber < 0 || RoundNumber >= RoundData.SingleRoundInfos.Num())
{
    return 0xff;
}
return RoundData.SingleRoundInfos[RoundNumber].RoundTeamWinner;
