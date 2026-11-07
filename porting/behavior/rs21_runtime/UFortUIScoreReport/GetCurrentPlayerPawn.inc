// Original body 0x1450e5c20 (thunk 0x145318cd0). Offset evidence: LocalPlayer +0x28 (reflected UFortLocalPlayer*); CachedScoreReports Data +0x310 / Num +0x318 (reflected); FFortPlayerScoreReport stride 0x2a8 with PlayerState weak pointer at +0x38 (reflected).
// Index below 0 or not below Num returns null (TEST EDX/JS, CMP EDX/JGE). Weak Get is the KNOWN helper 0x142ee5dd0; the tail call is AFortPlayerState::GetCurrentPawn (0x1419b6d20).
// CONFIDENCE: HIGH
if (LocalPlayer == nullptr)
{
    return nullptr;
}
if (ScoreReportIndex < 0 || ScoreReportIndex >= LocalPlayer->CachedScoreReports.Num())
{
    return nullptr;
}
AFortPlayerState* PlayerState = LocalPlayer->CachedScoreReports[ScoreReportIndex].PlayerState.Get();
if (PlayerState == nullptr)
{
    return nullptr;
}
return PlayerState->GetCurrentPawn();
