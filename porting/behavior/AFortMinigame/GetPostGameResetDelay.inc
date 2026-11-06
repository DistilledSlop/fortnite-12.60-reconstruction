// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
// Native 0x142093650; wrapper 0x1429c0cb0.
float Delay = 0.0f;
if (TotalRounds > 1)
{
    Delay = RoundScoreDisplayTime;
    Delay += RoundWinnerDisplayTime;
}
if (EndMethod == EFortMinigameEnd::EndGame || (EndMethod == EFortMinigameEnd::EndRound && IsLastRound()))
{
    Delay += GameWinnerDisplayTime;
    Delay += GameScoreDisplayTime;
}
if (FMath::Abs(Delay) > 1.0e-8f) Delay += PostGameResetDelay;
return Delay;
