// CONFIDENCE: high; captured native flow and typed dependency behavior.
// Original address: 0x142794370
// Native 0x141663120 queries active AI pawns within AIRelevantDistanceToPlayer,
// skips sleeping pawns (bit 12 at offset 0x18dc: bIsSleeping), and returns true
// if any alive non-sleeping AI is within distance of the player.
if (!Player)
{
    return false;
}

const FVector PlayerLocation = Player->GetActorLocation();
const float MaxDistSq = AIRelevantDistanceToPlayer * AIRelevantDistanceToPlayer;

for (const AFortAIPawn* AIPawn : AIPawns)
{
    if (AIPawn && !AIPawn->bIsSleeping)
    {
        if (FVector::DistSquared(AIPawn->GetActorLocation(), PlayerLocation) <= MaxDistSq)
        {
            return true;
        }
    }
}

return false;
