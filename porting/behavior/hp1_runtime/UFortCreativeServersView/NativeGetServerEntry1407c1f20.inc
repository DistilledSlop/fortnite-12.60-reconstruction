// Helper 0x1407c1f20, host UFortCreativeServersView (static).
// Evidence: if Index < 0 or Index >= int32 Num at +0xb8, return null; otherwise tail-jump to TWeakObjectPtr::Get on
// the 8-byte element at Data(+0xb0)[Index]. Callers: NavigateServerLeft/Right, HandleLeft/RightTabPressed, AttemptToUpdateFocus, GetFirstEquipSlotButton.
// CONFIDENCE: HIGH
const uint8* ContextRaw = reinterpret_cast<const uint8*>(Context);
const int32 EntryCount = *reinterpret_cast<const int32*>(ContextRaw + 0xb8);
if (Index < 0 || Index >= EntryCount)
{
    return nullptr;
}
TWeakObjectPtr<UObject>* EntryArray = *reinterpret_cast<TWeakObjectPtr<UObject>* const*>(ContextRaw + 0xb0);
return EntryArray[Index].Get();
