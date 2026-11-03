// Native body 0x141a489b0. CONFIDENCE: high
if (CurrentCheckPoint >= SplineCheckPointIndices.Num())
{
    return -1;
}
return SplineCheckPointIndices[FMath::Min(static_cast<int32>(CurrentCheckPoint) + 1, SplineCheckPointIndices.Num() - 1)];
