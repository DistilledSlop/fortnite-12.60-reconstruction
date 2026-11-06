// CONFIDENCE: high; captured tag-query evaluation, array order and null behavior.
{
    const int32 SourceCount = PlayerStartComponents.Num();
    OutPlayerStartComponents.Reset();
    OutPlayerStartComponents.Reserve(SourceCount);
    for (UFortMinigamePlayerStartComponent* Component : PlayerStartComponents)
    {
        if (TagQuery.Matches(Component->MinigameTags))
        {
            OutPlayerStartComponents.Add(Component);
        }
    }
}
