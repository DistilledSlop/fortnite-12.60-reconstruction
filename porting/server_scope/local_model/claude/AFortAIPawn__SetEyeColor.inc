// CONFIDENCE: MEDIUM. Applies the eye color and brightness to the eye material instance (EmissiveColor / EmissiveBrightness), creating the dynamic instance when the slot has none.
// Original address: 0x1427a5ed0 (exec). Virtual target: 0x1422ec440 (vtable slot 0xe60); material create 0x1440e4530 (slot 0x560), GetMaterial slot 0x548, GetNumMaterials slot 0x6a0.
if (Mesh == nullptr || EyeIndex < 0)
{
    return;
}
if (EyeIndex >= Mesh->GetNumMaterials())
{
    return;
}

UMaterialInstanceDynamic* EyeMaterial = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(EyeIndex));
if (!EyeMaterial)
{
    EyeMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(EyeIndex);
}
if (EyeMaterial)
{
    static const FName EmissiveColorName(TEXT("EmissiveColor"));
    static const FName EmissiveBrightnessName(TEXT("EmissiveBrightness"));
    EyeMaterial->SetVectorParameterValue(EmissiveColorName, NewEyeColor);
    EyeMaterial->SetScalarParameterValue(EmissiveBrightnessName, NewEyeBrightness);
}
