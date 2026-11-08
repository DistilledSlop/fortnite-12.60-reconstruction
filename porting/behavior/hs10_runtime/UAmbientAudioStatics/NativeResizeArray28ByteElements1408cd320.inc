// Helper 0x1408cd320 (9 callers; RCX=&TArray{Data,Num,Max}, EDX=required count, R8D=previous Max).
// Asm: NewMax = RequiredNum ? QuantizeSize(RequiredNum*0x1c,0)/0x1c (clamped to MAX_int32 when below RequiredNum) : 0; if NewMax != PreviousMax and (Data||NewMax) Data=Realloc(Data,NewMax*0x1c,0); Max=NewMax.
// 0x142bfce10 = FMemory::QuantizeSize, 0x142bfd200 = FMemory::Realloc. Data at +0, Max at +0xc.
// CONFIDENCE: MEDIUM
void*& Data = *reinterpret_cast<void**>(ArrayMemory);
int32& MaxSlots = *reinterpret_cast<int32*>(ArrayMemory + 0xc);
int32 NewMax = 0;
if (RequiredNum != 0)
{
	const uint64 QuantizedBytes = static_cast<uint64>(FMemory::QuantizeSize(static_cast<SIZE_T>(static_cast<int64>(RequiredNum)) * 0x1c, 0));
	NewMax = static_cast<int32>(QuantizedBytes / 0x1c);
	if (RequiredNum > NewMax)
	{
		NewMax = MAX_int32;
	}
}
if (NewMax != PreviousMax)
{
	if (Data != nullptr || NewMax != 0)
	{
		Data = FMemory::Realloc(Data, static_cast<SIZE_T>(static_cast<int64>(NewMax)) * 0x1c, 0);
	}
}
MaxSlots = NewMax;
