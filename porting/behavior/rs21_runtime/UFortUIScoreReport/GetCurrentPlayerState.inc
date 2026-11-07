// Original body 0x1450e5c70 (thunk 0x145318d60). Offset evidence: LocalPlayer +0x28 (reflected UFortLocalPlayer*); CachedScoreReports Data +0x310 / Num +0x318 (reflected); FFortPlayerScoreReport stride 0x2a8 with PlayerState weak pointer at +0x38 (reflected).
// Index below 0 or not below Num returns null (TEST EDX/JS, CMP EDX/JGE). The weak Get is the KNOWN helper 0x142ee5dd0 and is the tail call of the body.
// CONFIDENCE: HIGH
if (LocalPlayer == nullptr)
{
    return nullptr;
}
if (ScoreReportIndex < 0 || ScoreReportIndex >= LocalPlayer->CachedScoreReports.Num())
{
    return nullptr;
}
return LocalPlayer->CachedScoreReports[ScoreReportIndex].PlayerState.Get();
