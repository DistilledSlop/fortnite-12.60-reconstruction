// Helper 0x1406909c0 (UFortUINavigationManager static, 119 callers).
// Arguments: RCX = &Array (Data at +0x0, Max at +0xC), EDX = RequestedCount, R8D = CurrentMax. Element size 48 (LEA*3, SHL 4).
// Count*48 is quantized by FMemory::QuantizeSize (0x142bfce10); the capacity is quantized/48, or INT32_MAX if the
// request exceeds it. Realloc (0x142bfd200) runs when the capacity changes and the array has data or a nonzero capacity.
// CONFIDENCE: HIGH
uint8* ArrayBase = static_cast<uint8*>(ArrayStorage);
void** ElementData = reinterpret_cast<void**>(ArrayBase);
int32* ElementMaxSlot = reinterpret_cast<int32*>(ArrayBase + 0xC);
int32 NewCapacity = RequestedCount;
if (RequestedCount != 0)
{
	const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(RequestedCount) * 48, 0);
	const int32 QuantizedCount = static_cast<int32>(QuantizedBytes / 48);
	NewCapacity = (RequestedCount > QuantizedCount) ? 0x7fffffff : QuantizedCount;
}
if (NewCapacity != CurrentMax)
{
	void* CurrentElements = *ElementData;
	if (CurrentElements != nullptr || NewCapacity != 0)
	{
		*ElementData = FMemory::Realloc(CurrentElements, static_cast<SIZE_T>(static_cast<int64>(NewCapacity)) * 48, 0);
	}
}
*ElementMaxSlot = NewCapacity;
