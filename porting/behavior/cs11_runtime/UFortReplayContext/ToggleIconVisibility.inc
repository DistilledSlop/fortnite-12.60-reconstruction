// Original body 0x14248cf10 (thunk 0x142ac75c0): UniqueMapIconVisibility (reflected TMap at +0x280) is searched with the ported helper 0x1407bda90. The bool value is at element +8 of the 24-byte set element (element index * 24 from the data pointer at +0x280). The flag is set to (old == 0) and the new value is returned. The asm dereferences the entry when the key is absent; this port returns false there (crash path, no state change).
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerControllerAthena) > 0, "AFortPlayerControllerAthena declares the hash table find helper");
int32 EntryIndex = -1;
AFortPlayerControllerAthena::NativeHashTableFindIndex1407bda90(&UniqueMapIconVisibility, EntryIndex, reinterpret_cast<uint64>(ToToggle.Get()));
if (EntryIndex == -1)
{
    return false;
}
uint8* MapElements = *reinterpret_cast<uint8**>(&UniqueMapIconVisibility);
uint8* EntryFlag = MapElements + static_cast<int64>(EntryIndex) * 24 + 8;
uint8 NewFlagValue = (*EntryFlag == 0) ? 1 : 0;
*EntryFlag = NewFlagValue;
return NewFlagValue != 0;
