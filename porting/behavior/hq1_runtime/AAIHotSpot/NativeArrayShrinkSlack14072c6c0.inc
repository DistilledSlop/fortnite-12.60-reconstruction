// Native helper 0x14072c6c0 (AAIHotSpot static). TArray shrink-to-slack, element size 8 bytes.
// Layout from asm: Data at +0x0, Num int32 at +0x8, Max int32 at +0xc.
// Slack = Max-Num; SlackBytes = (int64)Slack*8. Skip-to-keep when SlackBytes<0x4000 and 3*Num>=2*Max (keeps Max).
// Else if Slack>0x40 and Num>0: NewMax = FMemory::QuantizeSize(Num*8,0)>>3; Slack<=0x40 and Num!=0: keep Max; else NewMax=Num.
// If NewMax != Max: store Max; if Data!=0 or NewMax!=0, Data = Realloc(Data, NewMax*8, 0).
// CONFIDENCE: MEDIUM
uint8* ArrayBytes = reinterpret_cast<uint8*>(ArrayStorage);
const int32 PrevNum = *reinterpret_cast<const int32*>(ArrayBytes + 0x8);
const int32 PrevMax = *reinterpret_cast<const int32*>(ArrayBytes + 0xc);
const int32 SlackElems = PrevMax - PrevNum;
const uint64 SlackBytes = static_cast<uint64>(static_cast<int64>(SlackElems)) << 3;
int32 NewElemMax = PrevNum;
if (SlackBytes < 0x4000 && static_cast<int32>(PrevNum * 3) >= static_cast<int32>(PrevMax * 2))
{
	NewElemMax = PrevMax;
}
else if (SlackElems > 0x40)
{
	if (PrevNum > 0)
	{
		const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(static_cast<int64>(PrevNum)) * 8, 0);
		NewElemMax = static_cast<int32>(QuantizedBytes >> 3);
	}
}
else if (PrevNum != 0)
{
	NewElemMax = PrevMax;
}
if (NewElemMax != PrevMax)
{
	*reinterpret_cast<int32*>(ArrayBytes + 0xc) = NewElemMax;
	void* DataPtr = *reinterpret_cast<void**>(ArrayBytes);
	if (DataPtr != nullptr || NewElemMax != 0)
	{
		*reinterpret_cast<void**>(ArrayBytes) = FMemory::Realloc(DataPtr, static_cast<SIZE_T>(static_cast<int64>(NewElemMax)) * 8, 0);
	}
}
