// Body 0x1423486d0 (thunk 0x142a1f4c0). Same TArray<8-byte> at object+0xfb8 (Data), Num +0xfc0 (recovered gap members, see AddWeaponTraceIgnoreActor).
// Compare: UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0 (0x1407327b0). Stable removal of every equivalent entry; the run-based memmove (0x1458092f3) is expressed as the in-place compaction loop. Num written once at the end (asm: MOV [R13+0xfc0],R14D).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UCreativeQuestComponent) > 0, "UCreativeQuestComponent include for NativeWeakObjectPtrEquivalent1407327b0");
if (!ActorToRemove)
{
    return;
}
TWeakObjectPtr<UObject> Weak;
Weak = ActorToRemove;
TArray<TWeakObjectPtr<UObject>>& IgnoreList = *reinterpret_cast<TArray<TWeakObjectPtr<UObject>>*>(&NativeRecoveredUint64_0xfb8);
int32 Kept = 0;
for (int32 i = 0; i < IgnoreList.Num(); ++i)
{
    if (!UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&IgnoreList[i]), reinterpret_cast<const uint8*>(&Weak)))
    {
        IgnoreList[Kept] = IgnoreList[i];
        ++Kept;
    }
}
NativeRecoveredInt32_0xfc0 = Kept;
