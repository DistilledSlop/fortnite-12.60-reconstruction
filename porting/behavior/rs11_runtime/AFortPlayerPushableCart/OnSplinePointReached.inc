// Behaviour snippet: AFortPlayerPushableCart::OnSplinePointReached (body only).
// Original address: 0x141a54650 (thunk 0x142a89840)
// Offset evidence: this+0x828 = CurrentCheckPoint (reflected uint8); this+0x818 = SplineCheckPointIndices (reflected TArray<int32>, Num at +0x820);
// this+0x790 = OnCheckPointReached (reflected FOnCheckPointReached, one int32 param). Stored byte at +0x828 is also the broadcast value.
// CONFIDENCE: HIGH
if (bWasMovingForward)
{
    const int32 NextIndex = static_cast<int32>(CurrentCheckPoint) + 1;
    if (NextIndex < SplineCheckPointIndices.Num() && SplinePointReached >= SplineCheckPointIndices[NextIndex])
    {
        CurrentCheckPoint = static_cast<uint8>(NextIndex);
        OnCheckPointReached.Broadcast(NextIndex);
    }
}
