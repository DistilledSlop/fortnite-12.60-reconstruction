// Original body address: 0x141aae350 (thunk 0x1427e1200)
// Offset evidence: RDX = TestList (TArray<FAshtonStoneState>&). Num at +0x8 (MOVSXD R8,[RDX+0x8]); Data at +0x0 (MOV RDX,[RDX]).
// Element stride 0x18 (LEA RDX,[RDX+0x18]); byte at element+0x1 is StoneState (FAshtonStoneState offset 1), compared with 0x2.
// LEA/CMOVNZ increments the count only on match. Loop label is the CMP after the single INC (JNZ 0x141aae361 in the full asm).
// CONFIDENCE: HIGH
int32 Count = 0;
for (int32 i = 0; i < TestList.Num(); ++i)
{
    if (static_cast<uint8>(TestList[i].StoneState) == 2)
    {
        ++Count;
    }
}
return Count;
