// Helper 0x1406d9890 (9 callers; RCX=&TArray{Data,Num,Max} with Num already updated).
// Asm: Grow = Max ? Num+Num*3/8 : max(Num,1); NewMax = QuantizeSize(Grow<<6,0)>>6 (MAX_int32 if Num > NewMax); Max=NewMax; if (Data||NewMax) Data=Realloc(Data,NewMax<<6,0).
// 0x142bfce10 = FMemory::QuantizeSize, 0x142bfd200 = FMemory::Realloc. Data +0, Num +8, Max +0xc.
// CONFIDENCE: MEDIUM
void*& Data = *reinterpret_cast<void**>(ArrayMemory);
const int32 CurrentNum = *reinterpret_cast<const int32*>(ArrayMemory + 0x8);
int32& MaxSlots = *reinterpret_cast<int32*>(ArrayMemory + 0xc);
const uint64 NumWide = static_cast<uint64>(static_cast<int64>(CurrentNum));
uint64 Grow = 1;
if (MaxSlots != 0)
{
	Grow = ((NumWide * 3) >> 3) + NumWide;
}
else if (NumWide > Grow)
{
	Grow = NumWide;
}
int32 NewMax = static_cast<int32>(static_cast<uint64>(FMemory::QuantizeSize(static_cast<SIZE_T>(Grow << 6), 0)) >> 6);
if (CurrentNum > NewMax)
{
	NewMax = MAX_int32;
}
MaxSlots = NewMax;
if (Data != nullptr || NewMax != 0)
{
	Data = FMemory::Realloc(Data, static_cast<SIZE_T>(static_cast<int64>(NewMax)) << 6, 0);
}
