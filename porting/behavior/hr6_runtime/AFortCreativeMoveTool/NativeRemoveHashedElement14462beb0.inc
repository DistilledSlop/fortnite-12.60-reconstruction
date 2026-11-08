// Helper 0x14462beb0 (AFortCreativeMoveTool), 97 instructions. Removes the element whose 8-byte key equals *KeyInput
// from a TSet-style container (sparse element array + hash buckets), returns true when an element was found.
// Layout (raw, from asm): +0x00 Elements data ptr (16-byte elements: int32 key-lo/prev-free @+0, NextFree @+4,
// key uint64 @+0, HashNextId @+8, HashIndex @+0xc), +0x08 Elements count, +0x20 alloc-flags heap ptr (inline at +0x10),
// +0x30 FirstFreeIndex, +0x34 NumFreeIndices, +0x38 inline bucket buffer, +0x40 heap bucket ptr, +0x48 HashSize.
// Hash = (int32)(hi32*0x17 + lo32) & (HashSize-1). Empty set (count == NumFree) returns false.
// Caller 14462c4a0 computes RCX = container (RSI+8 or RDX) and RDX = stack key pointer.
// Bucket walk is unlinked only when the element is found in the chain; the slot is released in either case
// (the asm falls through to the release block when the chain ends without a match).
// CONFIDENCE: MEDIUM
uint8* SetBytes = reinterpret_cast<uint8*>(SetContainer);
const int32 ElemCount = *reinterpret_cast<int32*>(SetBytes + 0x08);
const int32 FreeCount = *reinterpret_cast<int32*>(SetBytes + 0x34);
if (ElemCount == FreeCount)
{
	return false;
}
uint8* SetElems = *reinterpret_cast<uint8**>(SetBytes + 0x00);
uint8* HeapBuckets = *reinterpret_cast<uint8**>(SetBytes + 0x40);
uint8* BucketHeads = (HeapBuckets != nullptr) ? HeapBuckets : (SetBytes + 0x38);
const int32 HashSizeValue = *reinterpret_cast<int32*>(SetBytes + 0x48);
const int64 BucketMask = static_cast<int64>(HashSizeValue) - 1;

const uint64 KeyValue = *KeyInput;
const uint32 KeyHi = static_cast<uint32>(KeyValue >> 32);
const uint32 KeyLo = static_cast<uint32>(KeyValue);
const int32 HashValue = static_cast<int32>(KeyHi * 0x17u + KeyLo);
const int64 StartBucket = static_cast<int64>(HashValue) & BucketMask;

int32 ElemIndex = *reinterpret_cast<int32*>(BucketHeads + StartBucket * 4);
if (ElemIndex == -1)
{
	return false;
}
while (ElemIndex != -1 && *reinterpret_cast<const uint64*>(SetElems + static_cast<int64>(ElemIndex) * 16) != KeyValue)
{
	ElemIndex = *reinterpret_cast<int32*>(SetElems + static_cast<int64>(ElemIndex) * 16 + 8);
}
if (ElemIndex == -1)
{
	return false;
}

// Found: ElemIndex is the element index. Its stored HashIndex selects the bucket to unlink from.
const int64 FoundBucket = static_cast<int64>(*reinterpret_cast<int32*>(SetElems + static_cast<int64>(ElemIndex) * 16 + 0xc)) & BucketMask;
int32* UnlinkSlot = reinterpret_cast<int32*>(BucketHeads + FoundBucket * 4);
int32 ChainCur = *UnlinkSlot;
while (ChainCur != -1 && ChainCur != ElemIndex)
{
	UnlinkSlot = reinterpret_cast<int32*>(SetElems + static_cast<int64>(ChainCur) * 16 + 8);
	ChainCur = *UnlinkSlot;
}
if (ChainCur == ElemIndex)
{
	*UnlinkSlot = *reinterpret_cast<int32*>(SetElems + static_cast<int64>(ElemIndex) * 16 + 8);
}

// Release the sparse slot: push onto the free list.
const uint32 SlotBit = 1u << (static_cast<uint32>(ElemIndex) & 31u);
if (FreeCount != 0)
{
	const int32 OldFirstFree = *reinterpret_cast<int32*>(SetBytes + 0x30);
	*reinterpret_cast<int32*>(SetElems + static_cast<int64>(OldFirstFree) * 16) = ElemIndex;
}
*reinterpret_cast<int32*>(SetElems + static_cast<int64>(ElemIndex) * 16) = -1;
int32 NextFreeValue = -1;
if (FreeCount > 0)
{
	NextFreeValue = *reinterpret_cast<int32*>(SetBytes + 0x30);
}
*reinterpret_cast<int32*>(SetElems + static_cast<int64>(ElemIndex) * 16 + 4) = NextFreeValue;

uint8* FlagBase = SetBytes + 0x10;
uint8* FlagHeap = *reinterpret_cast<uint8**>(SetBytes + 0x20);
if (FlagHeap != nullptr)
{
	FlagBase = FlagHeap;
}
uint32* FlagWords = reinterpret_cast<uint32*>(FlagBase);
FlagWords[static_cast<uint32>(ElemIndex) >> 5] &= ~SlotBit;

*reinterpret_cast<int32*>(SetBytes + 0x34) = FreeCount + 1;
*reinterpret_cast<int32*>(SetBytes + 0x30) = ElemIndex;
return true;
