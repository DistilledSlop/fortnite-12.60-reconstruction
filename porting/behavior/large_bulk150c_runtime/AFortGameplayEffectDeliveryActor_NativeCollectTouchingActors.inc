// Native 0x14269fc60. CONFIDENCE: high
TInlineComponentArray<UPrimitiveComponent*> Components;
GetComponents(Components);
struct FTouchCandidate
{
    AActor* Actor;
    float DistanceSquared;
};
TArray<FTouchCandidate, TInlineAllocator<16>> Candidates;
const FVector Origin = GetActorLocation();
for (UPrimitiveComponent* Component : Components)
{
    if (!Component || !Component->GetGenerateOverlapEvents())
    {
        continue;
    }
    TArray<UPrimitiveComponent*> Overlapping;
    Component->GetOverlappingComponents(Overlapping);
    for (UPrimitiveComponent* Other : Overlapping)
    {
        if (Other && Other->GetGenerateOverlapEvents())
        {
            AActor* Owner = Other->GetOwner();
            if (Owner)
            {
                FTouchCandidate Candidate = {Owner, FVector::DistSquared(Origin, Owner->GetActorLocation())};
                Candidates.Add(Candidate);
            }
        }
    }
}
Candidates.Sort([](const FTouchCandidate& A, const FTouchCandidate& B) { return A.DistanceSquared < B.DistanceSquared; });
for (const FTouchCandidate& Candidate : Candidates)
{
    Actors.Add(Candidate.Actor);
}
