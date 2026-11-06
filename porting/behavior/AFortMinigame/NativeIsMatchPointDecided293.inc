// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
// Native 0x142099c10.
if (CurrentRound >= TotalRounds) return true;
TMap<int32, int32> Wins;
int32 Leader = INDEX_NONE;
for (int32 Winner : RoundWinHistory)
{
    if (Winner == INDEX_NONE) continue;
    int32& Count = Wins.FindOrAdd(Winner);
    Count = static_cast<int32>(uint32(Count) + 1u);
    if (Leader == INDEX_NONE || (Leader != Winner && Count > Wins.FindChecked(Leader))) Leader = Winner;
}
if (Leader == INDEX_NONE) return false;
const __m128 Total = _mm_cvtsi32_ss(_mm_setzero_ps(), TotalRounds);
const int32 Rounded = _mm_cvtss_si32(_mm_sub_ss(Total, _mm_set_ss(0.5f)));
const int32 Threshold = (Rounded >> 1) + 1;
const int32 LeaderWins = Wins.FindChecked(Leader);
if (LeaderWins >= Threshold) return true;
if (Wins.Num() <= 1) return false;
const int32 Remaining = static_cast<int32>(uint32(TotalRounds) - uint32(CurrentRound));
for (const TPair<int32, int32>& Win : Wins)
{
    if (Win.Key != Leader && static_cast<int32>(uint32(Win.Value) + uint32(Remaining)) >= LeaderWins) return false;
}
return true;
