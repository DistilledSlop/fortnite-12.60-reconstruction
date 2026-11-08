// Body 0x1423ae360 (thunk 0x142a4eac0). HiddenActors TArray<uint64> at controller +0x2e8 (Num +0x2f0, Max +0x2f4); APlayerController is an engine ancestor (not generated), so raw access at object offset.
// Add path: ULevelSaveBaseComponent::NativeAddUnique14085a990 (0x14085a990) with the target pointer bits; returns true. Remove path: removal of every matching entry (asm run-compaction with memmove 0x1458092f3 expressed as TArray::Remove); returns true when anything was removed.
// CONFIDENCE: MEDIUM
static_assert(sizeof(ULevelSaveBaseComponent) > 0, "ULevelSaveBaseComponent include for NativeAddUnique14085a990");
if (!TargetActor)
{
    return false;
}
const uint64 ActorBits = reinterpret_cast<uint64>(TargetActor);
TArray<uint64>& HiddenList = *reinterpret_cast<TArray<uint64>*>(reinterpret_cast<uint8*>(this) + 0x2e8);
if (bNewHidden)
{
    ULevelSaveBaseComponent::NativeAddUnique14085a990(HiddenList, &ActorBits);
    return true;
}
return HiddenList.Remove(ActorBits) > 0;
