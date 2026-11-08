// helper 0x140fd9650 (host UFortGlobals): sparse array clear with slack in EDX, element stride 0x38
// evidence: for each live element the buffers at +0x20 then +8 are freed (reverse member destruction order), Num=0, data resized to EDX (0x1406946d0, stride 0x38),
//   FirstFree=-1 (+0x30), NumFree=0 (+0x34), bit array emptied with capacity rounded up to 32 bits (0x140690310)
// CONFIDENCE: MEDIUM
struct FRawSparseElem
{
	uint64 LeadingValue;
	FString FirstText;
	uint64 MiddleValue;
	FString SecondText;
	int32 HashNextId;
	int32 HashIndex;
};
static_assert(sizeof(FRawSparseElem) == 0x38, "element size must match the 0x38 stride in the asm");
reinterpret_cast<TSparseArray<FRawSparseElem>*>(SparseArrayStorage)->Empty(ExpectedNumElements);
