// Helper 0x1406af460 (41 insns): RCX = destination struct, RDX = source struct, returns RCX.
// Callers (0x144b039c0, 0x14513a5d0, 0x144d01d10, ...) build the source on the stack as {ptr, int32 index (-1 default),
// int32 serial} followed by an array and assign it into a member: +0x0 qword, +0x8 dword, +0xc dword are copied
// verbatim (16 bytes), then the array at +0x10 (Data, Num at +8, Max at +0xc) is copied when it is a different array.
// Array part: Num copied, 0x1406d93e0 = ResizeAllocation(Num, PrevMax, 8) (QuantizeSize + Realloc, element size 8),
// then memcpy of Num*8 bytes = TArray<uint64> copy assignment. No reflected member covers these offsets.
// CONFIDENCE: MEDIUM (array element type recovered only as an 8-byte trivially copyable value)
FMemory::Memcpy(Dest, Source, 0x10);
TArray<uint64>& DestArray = *reinterpret_cast<TArray<uint64>*>(Dest + 0x10);
const TArray<uint64>& SourceArray = *reinterpret_cast<const TArray<uint64>*>(Source + 0x10);
if (&DestArray != &SourceArray)
{
    DestArray = SourceArray;
}
return Dest;
