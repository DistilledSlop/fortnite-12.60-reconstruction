// CONFIDENCE: MEDIUM. Applies the skin color to the skin material instance (SkinColorParameter). A non-zero color creates the dynamic instance when the slot has none; an all-zero color leaves the slot alone.
// Original address: 0x1427a6450 (exec). Virtual target: 0x1422ecf80 (vtable slot 0xe68). The target takes only the color; NewSkinBrightness is passed by the thunk but the target does not read it.
if (Mesh == nullptr || SkinIndex < 0)
{
    return;
}
if (SkinIndex >= Mesh->GetNumMaterials())
{
    return;
}

UMaterialInstanceDynamic* SkinMaterial = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(SkinIndex));
if (!SkinMaterial)
{
    const bool bHasColor = !(NewSkinColor.R == 0.0f && NewSkinColor.G == 0.0f && NewSkinColor.B == 0.0f && NewSkinColor.A == 0.0f);
    if (bHasColor)
    {
        SkinMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(SkinIndex);
    }
}
if (SkinMaterial)
{
    static const FName SkinColorParameterName(TEXT("SkinColorParameter"));
    SkinMaterial->SetVectorParameterValue(SkinColorParameterName, NewSkinColor);
}
