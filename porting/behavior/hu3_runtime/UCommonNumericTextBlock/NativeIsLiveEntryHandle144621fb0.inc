// Helper 0x144621fb0, host UCommonNumericTextBlock (17 callers; e.g. caller 0x142151040 passes RCX=result of CALL 0x140f95ae0 and RDX=&[RBP+0x760]).
// Offset evidence (raw; the container is not a reflected UObject member): [+0x08] entry data pointer (stride 0x90),
//   [+0x10] int32 Num, [+0x18] inline allocation-bit words, [+0x28] heap allocation-bit words pointer,
//   entry +0x70 uint64 Handle, entry +0x01 uint8 State (4 = dead).
// Asm returns the masked bit word (nonzero) on success; callers only test RAX for zero, so this returns bool.
// CONFIDENCE: MEDIUM
const uint64 Handle = *HandlePtr;
if (Handle == 0) { return false; }
const uint32 EntryIndex = static_cast<uint32>(Handle) & 0xffffffu;
if (static_cast<int32>(EntryIndex) >= *reinterpret_cast<const int32*>(SparseArrayPtr + 0x10)) { return false; }
const uint8* BitHeap = *reinterpret_cast<uint8* const*>(SparseArrayPtr + 0x28);
const uint8* BitWords = BitHeap ? BitHeap : (SparseArrayPtr + 0x18);
const uint32 BitWord = *reinterpret_cast<const uint32*>(BitWords + static_cast<uint64>(EntryIndex >> 5) * 4);
if ((BitWord & (1u << (EntryIndex & 31u))) == 0) { return false; }
const uint8* Entry = *reinterpret_cast<uint8* const*>(SparseArrayPtr + 0x08) + static_cast<uint64>(EntryIndex) * 0x90;
if (*reinterpret_cast<const uint64*>(Entry + 0x70) != Handle) { return false; }
return *(Entry + 0x01) != 4;
