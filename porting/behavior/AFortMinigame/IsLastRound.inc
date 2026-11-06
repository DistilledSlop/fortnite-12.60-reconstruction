// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
// Native 0x142099860; wrapper 0x1429c2e20.
if (CurrentState == EFortMinigameState::PostRoundEnd)
{
    return bEndGameOnMatchPointWin && NativeIsMatchPointDecided293();
}
if (CurrentState == EFortMinigameState::PostGameEnd || CurrentState == EFortMinigameState::PostGameAbandon) return true;
return bEndGameOnMatchPointWin ? NativeIsMatchPointDecided293() : CurrentRound >= TotalRounds;
