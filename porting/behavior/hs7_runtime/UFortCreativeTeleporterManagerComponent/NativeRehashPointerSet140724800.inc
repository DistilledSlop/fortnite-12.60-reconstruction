// helper 0x140724800 (host UFortCreativeTeleporterManagerComponent): TSet<void*>::Rehash (private in the engine, so done on raw storage)
// evidence: caller 0x141e3eaf0 passes this+8; elements Data@0 stride 0x10 {Key, HashNextId@8, HashIndex@0xc}; allocation flags inline@0x10 / ptr@0x20, NumBits@0x28;
//   hash inline@0x38 / secondary ptr@0x40, HashSize@0x48; secondary freed via 0x142bfd200 (Realloc to 0), resized by 0x140690920 (inline-1 allocator, 4-byte ids),
//   buckets filled with -1, then every live element gets the Bob Jenkins pointer hash of (Key >> 4) (PointerHash in TypeHash.h).
// CONFIDENCE: MEDIUM
uint8* SetRaw = static_cast<uint8*>(SetStorage);
void** HashSecondaryPtr = reinterpret_cast<void**>(SetRaw + 0x40);
const int32 BucketCount = *reinterpret_cast<const int32*>(SetRaw + 0x48);
if (*HashSecondaryPtr != nullptr)
{
	*HashSecondaryPtr = FMemory::Realloc(*HashSecondaryPtr, 0, 0);
}
if (BucketCount == 0)
{
	return;
}
if (BucketCount > 1)
{
	*HashSecondaryPtr = FMemory::Realloc(nullptr, static_cast<SIZE_T>(BucketCount) * 4, 0);
}
int32* BucketIds = (*HashSecondaryPtr != nullptr) ? static_cast<int32*>(*HashSecondaryPtr) : reinterpret_cast<int32*>(SetRaw + 0x38);
for (int32 BucketIdx = 0; BucketIdx < BucketCount; ++BucketIdx)
{
	BucketIds[BucketIdx] = -1;
}
const int32 NumBitsLive = *reinterpret_cast<const int32*>(SetRaw + 0x28);
void* FlagsSecondary = *reinterpret_cast<void**>(SetRaw + 0x20);
const uint32* FlagWords = (FlagsSecondary != nullptr) ? static_cast<const uint32*>(FlagsSecondary) : reinterpret_cast<const uint32*>(SetRaw + 0x10);
uint8* ElemBase = *reinterpret_cast<uint8**>(SetRaw);
for (int32 SlotIdx = 0; SlotIdx < NumBitsLive; ++SlotIdx)
{
	if ((FlagWords[SlotIdx >> 5] & (1u << (SlotIdx & 31))) == 0)
	{
		continue;
	}
	uint8* ElemRaw = ElemBase + static_cast<int64>(SlotIdx) * 0x10;
	const void* ElemKey = *reinterpret_cast<void* const*>(ElemRaw);
	const int32 BucketSlot = static_cast<int32>(PointerHash(ElemKey)) & (BucketCount - 1);
	*reinterpret_cast<int32*>(ElemRaw + 0xc) = BucketSlot;
	*reinterpret_cast<int32*>(ElemRaw + 0x8) = BucketIds[BucketSlot];
	BucketIds[BucketSlot] = SlotIdx;
}
