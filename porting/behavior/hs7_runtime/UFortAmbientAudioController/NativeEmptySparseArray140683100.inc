// helper 0x140683100 (host UFortAmbientAudioController): clears a sparse array (Data@0, flags@0x10 inline x4 + ptr@0x20, NumBits@0x28, FirstFree@0x30, NumFree@0x34)
// evidence: bit iteration over the allocation flags (0x140687d00 = set-bit iterator advance) frees the buffer at element+8 of each 0x20-byte element,
//   then Num=0 / resize to 0 (0x140690f60), FirstFree=-1, NumFree=0, bit array emptied (0x140690310) and the remaining buffers freed.
//   That is TSparseArray::Empty(0) for an element made of 8 bytes, an FString and the two hash link ints.
// CONFIDENCE: MEDIUM
struct FRawSparseElem
{
	uint64 LeadingValue;
	FString TextValue;
	int32 HashNextId;
	int32 HashIndex;
};
static_assert(sizeof(FRawSparseElem) == 0x20, "element size must match the 0x20 stride in the asm");
reinterpret_cast<TSparseArray<FRawSparseElem>*>(SparseArrayStorage)->Empty(0);
