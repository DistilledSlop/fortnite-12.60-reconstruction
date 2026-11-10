// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActorPlayerPropAttachment__DoesMeshComponentHaveVisibleMaterial.evidence.json for captures and notes.
// Original address: 0x142743d80 (exec). Body 0x141bd99e0.
// True when the mesh has a material slot that is either empty or resolves to an opaque/masked base material (blend mode below translucent).
if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr)
{
	return false;
}

const int32 NumMaterials = StaticMeshComponent->GetNumMaterials();
for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
{
	UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial(MaterialIndex);
	if (MaterialInterface == nullptr)
	{
		return true;
	}

	UMaterialInterface* BaseMaterial = MaterialInterface->GetMaterial();
	if (BaseMaterial == nullptr || BaseMaterial->GetBlendMode() < BLEND_Translucent)
	{
		return true;
	}
}

return false;
