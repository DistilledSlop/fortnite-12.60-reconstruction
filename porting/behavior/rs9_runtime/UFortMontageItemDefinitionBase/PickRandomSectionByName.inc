// Original body: 0x141d8d1e0 (thunk 0x1429f54c0)
// Offset evidence: PlayRandomSectionByName TArray at +0x5f0 (Data pointer) with Num dword at +0x5f8 (reflected Array member); element FSectionNameAndWeight {FName SectionName @+0, float SectionWeight @+8}, element stride 0xc. Num==0 returns zeroed FName, Num==1 returns element 0, otherwise weighted scan with fallback to element 0.
// CONFIDENCE: HIGH
const int32 Num = PlayRandomSectionByName.Num();
if (Num == 0)
{
    return FName();
}
if (Num == 1)
{
    return PlayRandomSectionByName[0].SectionName;
}
float Total = 0.0f;
for (int32 Index = 0; Index < Num; ++Index)
{
    Total += PlayRandomSectionByName[Index].SectionWeight;
}
float Target = Total * RandomNumber;
for (int32 Index = 0; Index < Num; ++Index)
{
    Target -= PlayRandomSectionByName[Index].SectionWeight;
    if (!(Target > 0.0f))
    {
        return PlayRandomSectionByName[Index].SectionName;
    }
}
return PlayRandomSectionByName[0].SectionName;
