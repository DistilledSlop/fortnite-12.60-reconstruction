// CONFIDENCE: HIGH
// Member body for NativeGetOrCreateMID(UMeshComponent* Mesh, int32 Slot).
// Mesh is required non-null by the native shared body.
UMaterialInterface* Material = Mesh->GetMaterial(Slot);
if (Mesh == StaticMeshComponent && Material)
{
    const TArray<FAnimatingMaterialPair>* Mappings = nullptr;
    if (DamageVisualsState == EFortDamageVisualsState::DamagedAndAnimating)
        Mappings = &AnimatingMaterialMappings;
    else if (DamageVisualsState == EFortDamageVisualsState::DamagedAndStatic)
        Mappings = &DamagedButNotAnimatingMaterialMappings;
    if (Mappings)
    {
        UMaterialInterface* Selected = Material;
        UMaterialInterface* PreviousOriginal = nullptr;
        for (const FAnimatingMaterialPair& Pair : *Mappings)
        {
            if (Material->IsDependent(Pair.Original) &&
                (Selected == Material ||
                 (Pair.Override && Pair.Original->IsDependent(PreviousOriginal))))
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
    return Existing;
UMaterialInterface* Parent = Existing && Existing->Parent ? Existing->Parent : Material;
UMaterialInstanceDynamic* Result = UMaterialInstanceDynamic::Create(Parent, this);
if (Parent != Material)
    Result->CopyParameterOverrides(Existing);
Mesh->SetMaterial(Slot, Result);
return Result;
