// Helper 0x1406946d0: TArray<56-byte element> capacity resize.
// Layout (asm evidence): Data at +0x0 (qword), Max at +0xc (dword). Num is not touched.
// QuantizeSize(Count*0x38, 0) gives the slack capacity (the MUL/SHR sequence is x/0x38 for the
// quantized byte count); a requested count above that slack saturates to INT32_MAX (CMOVG).
// CONFIDENCE: MEDIUM - the x/56 magic-multiply sequence is replaced by an exact division.
uint8* ArrayBytes = reinterpret_cast<uint8*>(ArrayRaw);
int32 NewMax = RequestedMax;
if (RequestedMax != 0)
{
    const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(static_cast<int64>(RequestedMax)) * 0x38, 0);
    NewMax = static_cast<int32>(QuantizedBytes / 0x38);
    if (RequestedMax > NewMax)
    {
        NewMax = 0x7fffffff;
    }
}
int32& MaxRef = *reinterpret_cast<int32*>(ArrayBytes + 0xc);
if (NewMax != MaxRef)
{
    MaxRef = NewMax;
    void*& DataRef = *reinterpret_cast<void**>(ArrayBytes);
    if (DataRef != nullptr || NewMax != 0)
    {
        DataRef = FMemory::Realloc(DataRef, static_cast<SIZE_T>(static_cast<int64>(NewMax)) * 0x38, 0);
    }
}
