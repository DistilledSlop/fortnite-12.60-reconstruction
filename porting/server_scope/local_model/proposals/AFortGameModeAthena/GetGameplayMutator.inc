// CONFIDENCE: HIGH -- native 0x141946800, predicate 0x141940b10.
UFortMutatorListComponent* Component = static_cast<UFortMutatorListComponent*>(
    FindComponentByClass(UFortMutatorListComponent::StaticClass()));
if (Component)
{
    for (AFortGameplayMutator* Mutator : Component->Mutators)
    {
        if (Mutator && Mutator->GameplayTagContainer.HasTag(MutatorTag))
        {
            return Mutator;
        }
    }
}
return nullptr;
