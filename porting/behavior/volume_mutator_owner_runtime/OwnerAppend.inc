// CONFIDENCE: HIGH
OutMutators.Reset();
UFortMutatorListComponent* Component = static_cast<UFortMutatorListComponent*>(
    FindComponentByClass(UFortMutatorListComponent::StaticClass()));
if (Component)
{
    OutMutators.Append(Component->Mutators);
}
