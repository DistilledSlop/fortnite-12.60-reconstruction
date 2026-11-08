// Helper 0x1445d47d0 (10 callers; e.g. 0x1445f1e10 passes sret buffer in RCX and the source array in RDX, then uses RAX).
// Asm: Out.Data=0; Out.Num=(End-Begin)/0x18; per element: qword at +0 copied, FString at +8 copied (Reserve Num, memcpy Num*2, Max=Num).
// Offset evidence: element stride 0x18 = 8-byte value at +0 plus FString (Data/Num/Max) at +8; no recorded class, local struct used.
// This is TArray<Elem> copy construction (FString copy keeps Num and Max equal to the source Num).
// CONFIDENCE: MEDIUM
struct FNativeKeyedTextElem
{
	uint64 Key;
	FString Text;
};
static_assert(sizeof(FNativeKeyedTextElem) == 0x18, "element stride must be 0x18");
const TArray<FNativeKeyedTextElem>& SourceElems = *reinterpret_cast<const TArray<FNativeKeyedTextElem>*>(SourceArray);
new (OutArray) TArray<FNativeKeyedTextElem>(SourceElems);
return OutArray;
