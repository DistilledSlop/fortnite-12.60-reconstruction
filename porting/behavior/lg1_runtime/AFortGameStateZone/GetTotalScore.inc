// Original body address: 0x141e9f2a0
// Evidence: ScoreCategory byte switch. 0 reads +0xdc8 (TotalCombatScore), 1 reads +0xdcc (TotalBuildingScore), 2 reads +0xdd0 (TotalUtilityScore); all reflected Int.
// Default path returns 0 after the verbosity-gated diagnostic log at call 0x142c54b70 (elided, no state change).
// CONFIDENCE: MEDIUM
switch (static_cast<uint8>(ScoreCategory))
{
    case 0: return TotalCombatScore;
    case 1: return TotalBuildingScore;
    case 2: return TotalUtilityScore;
    default: return 0;
}
