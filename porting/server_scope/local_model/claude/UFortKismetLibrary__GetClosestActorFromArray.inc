// CONFIDENCE: HIGH
// Native 0x14296c740 -> 0x141ef10f0: returns the array element nearest to Actor, comparing squared distances between
// root component locations (origin when there is no root). Null entries are skipped; ties keep the earlier element.
// Returns null for a null Actor or when nothing qualifies.
if (!Actor)
{
    return nullptr;
}
const FVector ActorLocation = Actor->GetRootComponent() ? Actor->GetRootComponent()->GetComponentLocation() : FVector::ZeroVector;
AActor* Closest = nullptr;
float ClosestDistanceSquared = TNumericLimits<float>::Max();
for (AActor* Candidate : ArrayOfActors)
{
    if (!Candidate)
    {
        continue;
    }
    const FVector CandidateLocation = Candidate->GetRootComponent() ? Candidate->GetRootComponent()->GetComponentLocation() : FVector::ZeroVector;
    const float DistanceSquared = FVector::DistSquared(CandidateLocation, ActorLocation);
    if (DistanceSquared < ClosestDistanceSquared)
    {
        ClosestDistanceSquared = DistanceSquared;
        Closest = Candidate;
    }
}
return Closest;
