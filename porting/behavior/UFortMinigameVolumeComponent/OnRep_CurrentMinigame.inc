// CONFIDENCE: high
// Native body at 0x1429df0c0 broadcasts OnMinigameSetup if CurrentMinigame is valid.
AFortMinigame* Minigame = CurrentMinigame;
if (IsValid(Minigame))
{
    OnMinigameSetup.Broadcast(CurrentMinigame);
}
