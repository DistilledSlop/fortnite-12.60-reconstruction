// Original body address: 0x1416295f0 (thunk 0x142710c70)
// Offset evidence: this+0x278 is an unreflected gap directly after NoSlotAssignees (0x268, 16 bytes). The body loads a qword table pointer there and passes &this+0x278 to NativeFindWeakKeySlot140972ca0; table entries are 20 bytes and the returned value is the int32 at entry+0x8.
// Key: operator=(AIOwner) into a temporary FWeakObjectPtr; its qword is the key passed to the helper.
// CONFIDENCE: MEDIUM
TWeakObjectPtr<UObject> OwnerWeak;
OwnerWeak = AIOwner;
const int64 Key = *reinterpret_cast<const int64*>(&OwnerWeak);
int32 OutIndex = 0;
NativeFindWeakKeySlot140972ca0(reinterpret_cast<uint8*>(const_cast<uint64*>(&NativeRecoveredPtr_0x278)), &OutIndex, Key);
if (OutIndex == -1)
{
    return -1;
}
uint8* Entry = reinterpret_cast<uint8*>(NativeRecoveredPtr_0x278) + static_cast<int64>(OutIndex) * 20;
if (Entry == nullptr)
{
    return -1;
}
return *reinterpret_cast<const int32*>(Entry + 8);
