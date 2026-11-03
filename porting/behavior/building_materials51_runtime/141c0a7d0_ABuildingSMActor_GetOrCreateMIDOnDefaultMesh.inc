// CONFIDENCE: HIGH
// Native entry 0x141c0a7d0, shared body 0x141c0a630; exec 0x142756550.
UStaticMeshComponent* Mesh = StaticMeshComponent;
if (!Mesh)
{
    return nullptr;
}

UMaterialInterface* Material = Mesh->GetMaterial(ElementIndex);
if (Mesh == StaticMeshComponent && Material)
{
    const TArray<FAnimatingMaterialPair>* Mappings = nullptr;
    if (DamageVisualsState == EFortDamageVisualsState::DamagedAndAnimating)
    {
        Mappings = &AnimatingMaterialMappings;
    }
    else if (DamageVisualsState == EFortDamageVisualsState::DamagedAndStatic)
    {
        Mappings = &DamagedButNotAnimatingMaterialMappings;
    }
    if (Mappings)
    {
        UMaterialInterface* Selected = Material;
        UMaterialInterface* PreviousOriginal = nullptr;
        for (const FAnimatingMaterialPair& Pair : *Mappings)
        {
            if (Material->IsDependent(Pair.Original) &&
                (Selected == Material || (Pair.Override && Pair.Original->IsDependent(PreviousOriginal))))
            {
                Selected = Pair.Override;
                PreviousOriginal = Pair.Original;
            }
        }
        Material = Selected;
    }
}

UMaterialInstanceDynamic* Existing = Cast<UMaterialInstanceDynamic>(Material);
if (Existing && Existing->GetOuter() == this)
{
    return Existing;
}
UMaterialInterface* Parent = Existing && Existing->Parent ? Existing->Parent : Material;
UMaterialInstanceDynamic* Result = UMaterialInstanceDynamic::Create(Parent, this);
if (Parent != Material)
{
    Result->CopyParameterOverrides(Existing);
}
Mesh->SetMaterial(ElementIndex, Result);
return Result;
