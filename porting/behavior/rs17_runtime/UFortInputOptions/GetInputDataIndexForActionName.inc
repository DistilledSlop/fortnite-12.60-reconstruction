// Original body address: 0x144e722e0 (thunk 0x1452605b0)
// Offset evidence: InputData TArray (Data +0x310, Num dword +0x318) loop; each element's ActionKeyMapping.ActionName (FFortActionKeyMapping at element +0x90, ActionName leading member: ComparisonIndex +0x90, Number +0x94) compared with ActionName; returns index or -1 (JLE and loop-exhausted paths).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortOptionsMenuInputData) > 0, "UFortOptionsMenuInputData must be a complete type here");
const int32 Count = InputData.Num();
for (int32 Index = 0; Index < Count; ++Index)
{
    if (InputData[Index]->ActionKeyMapping.ActionName == ActionName)
    {
        return Index;
    }
}
return -1;
