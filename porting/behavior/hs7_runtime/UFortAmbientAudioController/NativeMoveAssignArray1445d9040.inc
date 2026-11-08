// helper 0x1445d9040 (host UFortAmbientAudioController): move-assign for a raw TArray (Data@0, Num@8, Max@0xc) of 0x18-byte elements
// evidence: asm skips when RCX==RDX; each destination element's pointer at element+8 is passed to 0x142bee710 (FMemory::Free),
//   then the data buffer is freed, the source's Data/Num/Max are taken and the source Data, Num and Max are zeroed; returns RCX.
//   Element type is not reflected (no callers recorded), so elements are handled raw.
// CONFIDENCE: MEDIUM
if (DestArray == SrcArray)
{
	return DestArray;
}
uint8* DestRaw = static_cast<uint8*>(DestArray);
uint8* SrcRaw = static_cast<uint8*>(SrcArray);
uint8*& DestData = *reinterpret_cast<uint8**>(DestRaw);
int32& DestNum = *reinterpret_cast<int32*>(DestRaw + 8);
int32& DestMax = *reinterpret_cast<int32*>(DestRaw + 0xc);
uint8*& SrcData = *reinterpret_cast<uint8**>(SrcRaw);
int32& SrcNum = *reinterpret_cast<int32*>(SrcRaw + 8);
int32& SrcMax = *reinterpret_cast<int32*>(SrcRaw + 0xc);
for (int32 ElemIdx = 0; ElemIdx < DestNum; ++ElemIdx)
{
	void* ElemBuffer = *reinterpret_cast<void**>(DestData + static_cast<int64>(ElemIdx) * 0x18 + 8);
	if (ElemBuffer != nullptr)
	{
		FMemory::Free(ElemBuffer);
	}
}
if (DestData != nullptr)
{
	FMemory::Free(DestData);
}
DestData = SrcData;
SrcData = nullptr;
DestNum = SrcNum;
DestMax = SrcMax;
SrcNum = 0;
SrcMax = 0;
return DestArray;
