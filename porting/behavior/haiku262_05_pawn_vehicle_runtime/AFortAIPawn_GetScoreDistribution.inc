// CONFIDENCE: high
// Native0x1427a4cc0 (exec wrapper) -> native body 0x1422d72d0. Non-static member; reads OverriddenScoreDistribution (layout offset 0x1a00, Enum).
// Body: default 1 (EScoreDistributionType value 1); if the overridden value is nonzero, return it.
// Reflection's second Byte param "UnderlyingType" is not read by the native body.
EScoreDistributionType ScoreDistribution = static_cast<EScoreDistributionType>(1);
if (static_cast<uint8>(OverriddenScoreDistribution) != 0)
{
    ScoreDistribution = OverriddenScoreDistribution;
}
return ScoreDistribution;
