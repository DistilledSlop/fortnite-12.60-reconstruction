// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
// Native 0x142095660; wrapper 0x1429c1920.
outStateTimes.Reset();
const int32 Rounds = TotalRounds;
const bool bLastRound = IsLastRound();
const float Values[4] = { RoundWinnerDisplayTime, GameWinnerDisplayTime, RoundScoreDisplayTime, GameScoreDisplayTime };
for (int32 Index = 0; Index < 4; ++Index)
{
    const bool bEnabled = (Index == 0 || Index == 2) ? Rounds > 1 : bLastRound;
    outStateTimes.Add(bEnabled && FMath::Abs(Values[Index]) > 1.0e-8f ? Values[Index] : -1.0f);
}
