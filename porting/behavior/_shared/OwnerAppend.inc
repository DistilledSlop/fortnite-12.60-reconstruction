// CONFIDENCE: HIGH
OutMutators.Reset();
UFortMutatorListComponent* Component = MutatorListComponent;
if (Component)
{
    OutMutators.Append(Component->Mutators);
}
