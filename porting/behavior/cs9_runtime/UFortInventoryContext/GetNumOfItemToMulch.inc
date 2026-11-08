// Original body 0x144d06b20 (thunk 0x145260ae0). Offset evidence: sparse table at this+0x2c8 (NativeRecoveredPtr_0x2c8, 20-byte entries, count at entry+0x8), searched by the weak key of Item through NativeFindWeakKeySlot140972ca0 exactly as AAIHotSpot::GetSlotIndexByOwner does.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AAIHotSpot) > 0, "AAIHotSpot declares the slot-search helper");
TWeakObjectPtr<UFortItem> ItemWeak;
ItemWeak = Item;
const int64 Key = *reinterpret_cast<const int64*>(&ItemWeak);
int32 OutIndex = -1;
AAIHotSpot::NativeFindWeakKeySlot140972ca0(reinterpret_cast<uint8*>(const_cast<uint64*>(&NativeRecoveredPtr_0x2c8)), &OutIndex, Key);
if (OutIndex == -1)
{
    return 0;
}
const uint8* Entry = reinterpret_cast<const uint8*>(NativeRecoveredPtr_0x2c8) + static_cast<int64>(OutIndex) * 20;
if (Entry == nullptr)
{
    return 0;
}
return *reinterpret_cast<const int32*>(Entry + 8);
