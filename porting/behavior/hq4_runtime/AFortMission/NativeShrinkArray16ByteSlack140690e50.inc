// Helper 0x140690e50 (AFortMission static, 99 callers).
// Arguments: RCX = &Array (Data at +0x0, Num at +0x8, Max at +0xC), element size 16 (SHL 4).
// Keeps Max when the slack is under 0x4000 bytes and Num*3 >= Max*2; keeps Max when slack <= 64 and Num != 0;
// otherwise uses Num (when Num <= 0) or QuantizeSize(Num*16)>>4. Realloc (0x142bfd200) runs when Max changes and
// the array has data or a nonzero capacity. Quantize = 0x142bfce10, Realloc = 0x142bfd200.
// CONFIDENCE: HIGH
uint8* ArrayBase = static_cast<uint8*>(ArrayStorage);
void** ElementData = reinterpret_cast<void**>(ArrayBase);
const int32 ElemNum = *reinterpret_cast<const int32*>(ArrayBase + 0x8);
int32* ElemMaxSlot = reinterpret_cast<int32*>(ArrayBase + 0xC);
const int32 ElemMax = *ElemMaxSlot;
const int32 ElemSlack = ElemMax - ElemNum;
int32 NewCapacity = ElemMax;
const bool bSmallSlackBytes = static_cast<uint64>(static_cast<int64>(ElemSlack) * 16) < 0x4000ULL;
if (bSmallSlackBytes && ElemNum * 3 >= ElemMax * 2)
{
	NewCapacity = ElemMax;
}
else if (ElemSlack <= 64)
{
	NewCapacity = (ElemNum == 0) ? 0 : ElemMax;
}
else if (ElemNum <= 0)
{
	NewCapacity = ElemNum;
}
else
{
	NewCapacity = static_cast<int32>(FMemory::QuantizeSize(static_cast<SIZE_T>(ElemNum) * 16, 0) >> 4);
}
if (NewCapacity != ElemMax)
{
	*ElemMaxSlot = NewCapacity;
	void* CurrentElements = *ElementData;
	if (CurrentElements != nullptr || NewCapacity != 0)
	{
		*ElementData = FMemory::Realloc(CurrentElements, static_cast<SIZE_T>(static_cast<int64>(NewCapacity)) * 16, 0);
	}
}
