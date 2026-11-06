// CONFIDENCE: HIGH
// 0x141df6530. The const qualifier and source method name are assigned.
FString Type;
FString Name;
FPrimaryAssetId Id;
if (IdString.Split(TEXT(":"), &Type, &Name, ESearchCase::CaseSensitive, ESearchDir::FromStart))
{
    Id = FPrimaryAssetId(FPrimaryAssetType(FName(*Type)), FName(*Name));
}
UObject* Asset = GetPrimaryAssetObject(Id);
return Asset ? Asset : GetPrimaryAssetPath(Id).TryLoad();
