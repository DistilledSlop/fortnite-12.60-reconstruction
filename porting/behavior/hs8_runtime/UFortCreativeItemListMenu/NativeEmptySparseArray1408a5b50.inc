// helper 0x1408a5b50 (host UFortCreativeItemListMenu): same layout as helper 0x140683100 but with the slack count in EDX
// evidence: set-bit iteration frees the buffer at element+8 of every 0x20-byte element, Num=0, data resized to EDX (0x140690f60), FirstFree=-1, NumFree=0,
//   bit array NumBits=0 and capacity rounded up to a multiple of 32 bits from EDX (0x140690310)
// CONFIDENCE: MEDIUM
struct FRawSparseElem
{
	uint64 LeadingValue;
	FString TextValue;
	int32 HashNextId;
	int32 HashIndex;
};
static_assert(sizeof(FRawSparseElem) == 0x20, "element size must match the 0x20 stride in the asm");
reinterpret_cast<TSparseArray<FRawSparseElem>*>(SparseArrayStorage)->Empty(ExpectedNumElements);
