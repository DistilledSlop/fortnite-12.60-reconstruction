// CONFIDENCE: HIGH
// Original wrapper: 0x145300070; body reviewed against Gemini decompilation.
// GetActiveTab reads the recovered inherited FName; TabNameId is the reflected field.
if (PreregisteredTabInfoArray.Num() < 1)
{
    return false;
}
return GetActiveTab() == PreregisteredTabInfoArray.Last().TabNameId;
