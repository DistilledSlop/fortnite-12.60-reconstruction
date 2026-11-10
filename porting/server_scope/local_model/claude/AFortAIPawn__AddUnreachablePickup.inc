// CONFIDENCE: MEDIUM - exec 0x1427a44b0 calls body 0x1422bd060. The pickup is added to UnreachablePickups with the world stamp as value.
// Original addresses: 0x1427a44b0 (exec thunk) ; 0x1422bd060 (direct body)
if (Pickup == nullptr)
{
    return;
}
UWorld* World = GetWorld();
const int32 Stamp = World->FrameStamp540;
UnreachablePickups.Add(Pickup, Stamp);
