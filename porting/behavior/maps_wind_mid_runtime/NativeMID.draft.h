// Typed behavioral draft; not integrated or build verified.
#pragma once
#include "ABuildingSMActor.h"
#include "Components/MeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace FortWindMIDDraft
{
// Shared body 0x141c0a630. Mesh must be non-null, as in native code.
inline UMaterialInstanceDynamic* GetOrCreate(
    ABuildingSMActor& Actor, UMeshComponent* Mesh, int32 Slot)
{
    UMaterialInterface* Material = Mesh->GetMaterial(Slot);
    if (Mesh == Actor.StaticMeshComponent && Material)
    {
        const TArray<FAnimatingMaterialPair>* Mappings = nullptr;
        if (Actor.DamageVisualsState == EFortDamageVisualsState::DamagedAndAnimating)
            Mappings = &Actor.AnimatingMaterialMappings;
        else if (Actor.DamageVisualsState == EFortDamageVisualsState::DamagedAndStatic)
            Mappings = &Actor.DamagedButNotAnimatingMaterialMappings;
        // The native default array is empty, and the resolver returns null
        // immediately for null input; these checks preserve both results.
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
    if (Existing && Existing->GetOuter() == &Actor)
        return Existing;
    UMaterialInterface* Parent = Existing && Existing->Parent ? Existing->Parent : Material;
    UMaterialInstanceDynamic* Created = UMaterialInstanceDynamic::Create(Parent, &Actor);
    if (Parent != Material)
        Created->CopyParameterOverrides(Existing);
    Mesh->SetMaterial(Slot, Created);
    return Created;
}

// 0x1442ad650 is the Kismet library routine; its native parent-null check,
// world selection, optional name, and RF_Transient behavior belong to it.
inline UMaterialInstanceDynamic* Create(
    UObject* WorldContext, UMaterialInterface* Parent, FName Name,
    EMIDCreationFlags Flags)
{
    return UKismetMaterialLibrary::CreateDynamicMaterialInstance(
        WorldContext, Parent, Name, Flags);
}
}
