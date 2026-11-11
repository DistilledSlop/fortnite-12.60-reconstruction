// CONFIDENCE: LOW Original addresses: thunk 0x142b45560; see evidence.json for callees.
// Signature: void AttemptApplyOverrideWrapToActor(AActor* Actor)
// Native 0x142690600. Server or standalone only; clients receive applied wraps through replication.
if (!Actor || GetNetMode() == NM_Client)
{
    return;
}
// Skip when the weapon carries no override wrap data (native checks two zero-valued int fields).
if (NativeHasOverrideWrapData() == false)
{
    return;
}
// Entries copied from the owning pawn's wrap component, then applied to Actor.
TArray<FFortOverrideWrapEntry> WrapEntries = NativeGetOwnerWrapEntries();
NativeApplyItemWrapToActor(Actor, OverrideItemWrap, WrapEntries);
