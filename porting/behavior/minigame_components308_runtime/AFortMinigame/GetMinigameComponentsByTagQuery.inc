// CONFIDENCE: high; captured tag-query evaluation, array order and null behavior.
{
    const int32 SourceCount = MinigameComponents.Num();
    OutComponents.Reset();
    OutComponents.Reserve(SourceCount);
    for (UFortMinigameLogicComponent* Component : MinigameComponents)
    {
        if (Component && TagQuery.Matches(Component->MinigameTags))
        {
            OutComponents.Add(Component);
        }
    }
}
