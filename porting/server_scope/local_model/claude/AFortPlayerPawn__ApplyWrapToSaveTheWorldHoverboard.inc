// CONFIDENCE: LOW
// Original address: 0x142a7add0 (exec thunk body, no parameters). Decompile: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7add0.c (264 lines, read in full).
// Depth note: read to depth one and partly two. Read in full: FUN_142323510, FUN_140a003b0 (struct copy), FUN_141d4abb0, FUN_142a3d9f0, FUN_141d50e00, FUN_1407f2320, FUN_140682ce0.
// NOT read: FUN_141d85c80 (wrap-item resolver, 166 lines), FUN_144335300 (apply wrap to mesh, 407 lines), FUN_142ce9f90 (log formatting) and FUN_142cd3770 (STAT scope).
// The STAT scope is instrumentation (no gameplay effect) and is omitted. The log branches only write diagnostics.
USkeletalMeshComponent* Mesh = AttachmentMeshComponent;   // pawn +0x2278 (offset 8856)
if (Mesh == nullptr)
{
	return;
}
UObject* ChildComponent = Mesh->GetChildComponent(0);   // vslot 0x548, unnamed
bool bHasChildComponent = ChildComponent != nullptr && ChildComponent->IsAttachedQuery();   // vslot 0x450, unnamed
if (!bCosmeticLoadoutInitialized)   // CosmeticLoadout byte at offset 6664 (0x1a08), name not recovered
{
	if (LogVerbosity > 2)
	{
		LogWarning(TEXT("ApplyWrapToSaveTheWorldHoverboard: uninitialized cosmetic loadout, cannot apply wrap."));
	}
	return;
}
// FUN_142323510(this, 0) returns &CosmeticLoadout (+0x1918). Copy it into a local loadout.
FCosmeticLoadout Loadout = CosmeticLoadout;
UCosmeticItem* Item = Loadout.GetItemAt(0);   // FUN_141d4abb0(&Loadout, 0): cached item at +0xb8, else array element 0
if (Item != nullptr)
{
	// Controller (+0x250) cast to AFortPlayerController (FUN_142a3d9f0 = AFortPlayerController::StaticClass).
	AFortPlayerController* PC = IsA<AFortPlayerController>(Controller) ? Cast<AFortPlayerController>(Controller) : nullptr;
	// Collect the loadout's slot entries that reference Item (FUN_141d50e00, 0x48-byte entries, match at +0x30).
	TArray<FCosmeticSlotEntry> Matches = Loadout.FindEntriesForItem(Item);
	// Resolve the wrap item for the current child count (FUN_141d85c80, unread).
	UObject* WrapItem = ResolveHoverboardWrapItem(Item, bHasChildComponent ? 2 : 1, Matches, PC);
	if (WrapItem != nullptr && WrapItem != ChildComponent)
	{
		UObject* NewComponent = Mesh->SetChildComponent(0, WrapItem);   // vslot 0x568, unnamed
		if (NewComponent != nullptr)
		{
			ApplyWrapToMesh(NewComponent, ChildComponent, true);   // FUN_144335300, unread
		}
	}
	// Apply the loadout to the item (vslot 0x4e0 with index 0 and -1). The decompile passes -1 as the final slot index.
	Item->ApplyLoadout(0, Loadout, -1);
}
