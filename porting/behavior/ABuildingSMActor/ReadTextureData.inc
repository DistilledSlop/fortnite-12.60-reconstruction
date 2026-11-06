// CONFIDENCE: HIGH
// Native 0x141c183e0; reflected exec 0x142757e80.
TArray<UTexture2D*> Result;
for (UBuildingTextureData* Data : TextureData)
{
    Result.Add(Data ? Data->Diffuse : nullptr);
    Result.Add(Data ? Data->Normal : nullptr);
    Result.Add(Data ? Data->Specular : nullptr);
}
return Result;
