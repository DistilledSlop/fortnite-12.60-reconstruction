// Body 0x142302e10 (thunk 0x142a1af30). TArray of 8-byte FWeakObjectPtr at object+0xfb8 (Data), Num +0xfc0, Max +0xfc4; no reflected member covers these (lookup_offset AFortPawn), so recovered gap members are used.
// Compare: UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0 (0x1407327b0). Append path = TArray Add (grow helper 0x140690bb0).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UCreativeQuestComponent) > 0, "UCreativeQuestComponent include for NativeWeakObjectPtrEquivalent1407327b0");
if (!ActorToIgnore)
{
    return;
}
TWeakObjectPtr<UObject> Weak;
Weak = ActorToIgnore;
TArray<TWeakObjectPtr<UObject>>& IgnoreList = *reinterpret_cast<TArray<TWeakObjectPtr<UObject>>*>(&NativeRecoveredUint64_0xfb8);
for (int32 i = 0; i < IgnoreList.Num(); ++i)
{
    if (UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&IgnoreList[i]), reinterpret_cast<const uint8*>(&Weak)))
    {
        return;
    }
}
IgnoreList.Add(Weak);
