// CONFIDENCE: LOW. Chunk 01 port. See ABuildingGameplayActorPlayerPropAttachment__CalcWorldSpaceBounds.evidence.json for captures and notes.
// Original address: 0x142743bb0 (exec). Body 0x141bd7a30. Output parameters follow reflection offsets: Origin at +8, Extent at +20.
// Starts from the prop's root component bounds and grows the box to include the primitive components of attached children.
// With no root component, the output stays at the default constant from the decompile.
USceneComponent* Root = PropActor ? PropActor->GetRootComponent() : nullptr;
FBox Combined(ForceInit);
if (Root)
{
	Combined += Root->Bounds.GetBox();
	for (USceneComponent* Child : Root->GetAttachChildren())
	{
		if (!Child)
		{
			continue;
		}
		TArray<UPrimitiveComponent*> ChildPrimitives;
		CollectAttachedPrimitiveComponents(Child, ChildPrimitives);   // FUN_141bdcf00, de-duplicated
		for (UPrimitiveComponent* Primitive : ChildPrimitives)
		{
			Combined += Primitive->Bounds.GetBox();
		}
	}
}

Origin = Combined.GetCenter();
Extent = Combined.GetExtent();
