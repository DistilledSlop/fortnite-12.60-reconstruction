// CONFIDENCE: high
// Native target 0x142098c90.
for (UFortMinigameLogicComponent* Component : MinigameComponents)
{
    if (Component)
    {
        Component->HandlePlayspaceEditModeChange(bInEditMode);
    }
}
