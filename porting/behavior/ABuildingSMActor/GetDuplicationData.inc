// CONFIDENCE: HIGH
// Native 0x141c08050; reflected exec 0x1427564d0.
FBuildingDuplicationData Result;
Result.ClassData = GetClass();
for (int32 Index = 0; Index < 4; ++Index)
{
    Result.TextureData[Index] = TextureData[Index];
}
return Result;
