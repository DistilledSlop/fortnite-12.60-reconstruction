UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
if (ASC)
{
    for (int32 Index = 0; StaticGameplayTags.IsValidIndex(Index); ++Index)
    {
        // Reacquire the original element each iteration: OnTagUpdated is virtual.
        auto Tag = StaticGameplayTags.CreateConstIterator();
        for (int32 Skip = 0; Skip < Index; ++Skip)
        {
            ++Tag;
        }
        ASC->AddLooseGameplayTag(*Tag, 1);
    }
}
