// Behaviour snippet: UFortPickerContext::GetListPickerOption (body only).
// Original body address: 0x144d06270 (thunk 0x1452a49a0). this_delta_total 0.
// Offset evidence: this+0x90 is a byte picker type (no reflected member; class own region starts at 0x0, so rule 2 member NativeRecoveredByte_0x90); the body requires type 5 or 7 ((AL-5)&0xFD==0). this+0x94 is an int32 slot index (MOVSXD; NativeRecoveredInt32_0x94). this+0x98 is a pointer to 16-byte entries (NativeRecoveredPtr_0x98); entry Slot is {Data at +0, Num at +8}. The body returns null when Index >= Num (signed JGE), otherwise tail-jumps to the known helper 0x142ee5dd0 (TWeakObjectPtr<UObject>::Get) with RCX = &Data[Index] (8-byte weak pointer, Index*8).
// CONFIDENCE: MEDIUM
if (!(NativeRecoveredByte_0x90 == 5 || NativeRecoveredByte_0x90 == 7))
{
    return nullptr;
}
uint8* Entries = reinterpret_cast<uint8*>(NativeRecoveredPtr_0x98);
uint8* Entry = Entries + static_cast<int64>(NativeRecoveredInt32_0x94) * 16;
if (Index >= *reinterpret_cast<const int32*>(Entry + 8))
{
    return nullptr;
}
uint8* Data = *reinterpret_cast<uint8* const*>(Entry);
TWeakObjectPtr<UObject>* Weak = reinterpret_cast<TWeakObjectPtr<UObject>*>(Data + static_cast<int64>(Index) * 8);
return reinterpret_cast<UFortItem*>(Weak->Get());
