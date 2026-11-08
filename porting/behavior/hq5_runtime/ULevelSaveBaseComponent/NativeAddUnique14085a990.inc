// Helper 0x14085a990: TArray<8-byte element> AddUnique returning an index.
// Asm: qword scan from Data to Data+Num*8 comparing [RDX]; found returns index without modifying the array;
// not found increments Num (grow via 0x140690bb0 when Num+1 > Max) and stores the value at old Num, returning old Num.
// Confidence MEDIUM: the asm reads *Value after the grow; Add(*Value) reads it before, which only differs when Value aliases the array storage.
// CONFIDENCE: MEDIUM
const int32 OldNum = Array.Num();
for (int32 Index = 0; Index < OldNum; ++Index)
{
    if (Array[Index] == *Value)
    {
        return Index;
    }
}
Array.Add(*Value);
return OldNum;
