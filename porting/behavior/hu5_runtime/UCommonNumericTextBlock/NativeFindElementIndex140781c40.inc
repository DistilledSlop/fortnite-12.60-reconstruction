// Helper 0x140781c40 (host UCommonNumericTextBlock, static). Hashed find of a 64-bit key in a TSet-layout container; returns the element index or -1.
// Evidence: RCX = container, R8 = key qword, RDX = hidden int32 result. Caller 0x141c5fd20 @0x141c5fda6 (RCX=R12, RDX=[RSP+0x40]) reads the int32 at [RSP+0x40] after the call.
// Offsets: [+0x0] element data pointer, [+0x8] int32 compared to [+0x34] (early not-found exit), [+0x38] inline bucket array,
// [+0x40] bucket array pointer (used when non-null), [+0x48] int32 bucket count. Element stride 0x60: key qword at +0, chain-next int32 at +0x58.
// The 32-bit mixing sequence (0x9e3779b9 seed) is transcribed operation by operation from the asm; the meanings of +0x8 and +0x34 are not named.
// CONFIDENCE: MEDIUM
if (*reinterpret_cast<const int32*>(SetData + 0x8) == *reinterpret_cast<const int32*>(SetData + 0x34))
{
	return -1;
}
uint32 Mix9 = 0x9e3779b9u;
uint32 Mix10 = static_cast<uint32>(LookupKey >> 4);
uint32 MixCx = 0;
uint32 MixAx = 0;
Mix9 -= Mix10;
MixAx = Mix10 << 8;
Mix9 ^= MixAx;
MixAx = Mix9 >> 13;
MixCx = Mix9 + Mix10;
Mix10 -= Mix9;
MixCx = 0u - MixCx;
MixCx ^= MixAx;
Mix10 -= MixCx;
MixAx = MixCx >> 12;
Mix10 ^= MixAx;
Mix9 -= Mix10;
MixAx = Mix10;
Mix9 -= MixCx;
MixAx <<= 16;
Mix9 ^= MixAx;
MixCx -= Mix9;
MixAx = Mix9 >> 5;
MixCx -= Mix10;
MixCx ^= MixAx;
Mix10 -= Mix9;
Mix10 -= MixCx;
MixAx = MixCx >> 3;
Mix10 ^= MixAx;
Mix9 -= Mix10;
MixAx = Mix10;
Mix9 -= MixCx;
MixAx <<= 10;
Mix9 ^= MixAx;
MixCx -= Mix9;
uint64 BucketHash = static_cast<uint64>(Mix9) >> 15;
MixCx -= Mix10;
BucketHash ^= static_cast<uint64>(static_cast<int64>(static_cast<int32>(MixCx)));
const int64 BucketCount = static_cast<int64>(*reinterpret_cast<const int32*>(SetData + 0x48));
const uint64 BucketSlot = BucketHash & static_cast<uint64>(BucketCount - 1);
const uint8* HashArray = *reinterpret_cast<uint8* const*>(SetData + 0x40);
const uint8* BucketBase = HashArray != nullptr ? HashArray : SetData + 0x38;
int32 ElementIndex = *reinterpret_cast<const int32*>(BucketBase + BucketSlot * 4);
if (ElementIndex == -1)
{
	return -1;
}
const uint8* ElementBase = *reinterpret_cast<uint8* const*>(SetData);
while (true)
{
	const uint8* ElementPtr = ElementBase + static_cast<int64>(ElementIndex) * 0x60;
	if (*reinterpret_cast<const uint64*>(ElementPtr) == LookupKey)
	{
		return ElementIndex;
	}
	ElementIndex = *reinterpret_cast<const int32*>(ElementPtr + 0x58);
	if (ElementIndex == -1)
	{
		return -1;
	}
}
