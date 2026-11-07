// Original body address: 0x144d7b0d0 (thunk 0x1452207b0)
// Offset evidence: +0xb80 is UAthenaCreativeItemTileButton::ItemDefinition (reflected Object, inherited). The asm returns it only when it IsA UFortCreativeRealEstatePlotItemDefinition::StaticClass(), else null.
// CONFIDENCE: HIGH
if (ItemDefinition == nullptr)
{
	return nullptr;
}
UObject* Definition = ItemDefinition;
if (!Definition->IsA(UFortCreativeRealEstatePlotItemDefinition::StaticClass()))
{
	return nullptr;
}
return static_cast<UFortCreativeRealEstatePlotItemDefinition*>(Definition);
