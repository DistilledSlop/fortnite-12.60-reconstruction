// CONFIDENCE: high
// Native 0x14209a290; wrapper 0x1429c3170.
{
    if (Actor)
    {
        for (const UFortMinigameLogicComponent* Component : MinigameComponents)
        {
            if (Component && Component->GetOwner() == Actor)
            {
                return true;
            }
        }
    }
    return false;
}
