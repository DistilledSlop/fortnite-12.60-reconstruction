// Helper 0x14072a990: TSet<T>::Reset() for a TSet with default allocators (TSparseArray: Data @0, Num @8, Max @0xc,
// AllocationFlags TBitArray<Inline4> @0x10 (NumBits @0x28), FirstFreeIndex @0x30, NumFreeIndices @0x34; hash TInlineAllocator<1>
// inline @0x38, secondary @0x40, HashSize @0x48). Early-out when Data.Num == NumFreeIndices (set already empty); otherwise
// Data.Num = 0, bit array zeroed, FirstFreeIndex = -1, NumFreeIndices = 0, hash filled with INDEX_NONE (memset 0xff, HashSize*4).
// No element destructor loop is emitted, so T is trivially destructible and the element type does not affect the layout
// touched here; callers (AFortAthenaMutator_Bagel/Mash/Wax) pass the address of the set inside a per-player record.
TSet<UObject*>* TargetSet = reinterpret_cast<TSet<UObject*>*>(SetStorage);
TargetSet->Reset();
// CONFIDENCE: MEDIUM
