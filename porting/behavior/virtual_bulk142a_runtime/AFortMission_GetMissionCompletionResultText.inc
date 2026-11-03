// CONFIDENCE: high
// Native target 0x1420b9910.
switch (Result)
{
case EFortCompletionResult::Win: return NSLOCTEXT("MissionCompletion", "Win", "Victory!");
case EFortCompletionResult::Loss: return NSLOCTEXT("MissionCompletion", "Loss", "Defeat");
case EFortCompletionResult::Draw: return NSLOCTEXT("MissionCompletion", "Draw", "Draw");
default: return NSLOCTEXT("MissionCompletion", "Error", "Error!");
}
