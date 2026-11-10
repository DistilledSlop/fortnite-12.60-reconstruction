// CONFIDENCE: MEDIUM
// Original addresses: RPC thunk 0x1428d7390; _Validate virtual slot 0xab8 -> 0x1406801d0 (Stub_ReturnTrue);
// _Implementation virtual slot 0xac0 -> 0x1426b0d50 (body ported below).
// Declared parameter order (confirmed from thunk read order): Location, Rotation, AttachedActor, InBuildingAttachmentType.
if (!ServerSpawnDeco_Validate(Location, Rotation, AttachedActor, InBuildingAttachmentType))
{
	RPC_ValidateFailed(TEXT("ServerSpawnDeco_Validate"));
	return;
}

// ---- _Implementation (0x1426b0d50) ----
AFortPlayerController* InstigatorPC = GetInstigatorController();      // 0x14269e2e0
UFortDecoItemDefinition* DecoItemDef = GetDecoItemDefinition();        // virtual 0xb00 -> 0x14269bb00
UClass* BuildingClass = nullptr;
if (DecoItemDef != nullptr)
{
	// 0x142009ac0 -> 0x141ff85e0(out, DecoItemDef+0xad0, 1): first entry of the deco item's building-class list (0x141ff85e0 not captured here).
	BuildingClass = GetFirstDecoBuildingClass(DecoItemDef);
}

if (InstigatorPC != nullptr && DecoItemDef != nullptr && BuildingClass != nullptr
	&& BuildingClass->IsChildOf(ABuildingActor::StaticClass()))          // 0x142732ac0 = ABuildingActor::StaticClass
{
	// Placement settings shared reference (0x142c0c240 copies a shared ref from 0x1407c2c80, released at scope end).
	TSharedRef<FFortPlacementSettings> PlacementSettings = GetDefaultPlacementSettings();

	// Feasibility check (0x142663ee0, 583 lines, captured, NOT ported in this pass).
	// Returns 0 = not placeable, 1 = placeable, 2 = placeable only when the item is flagged (DecoItemDef+0xb70).
	UFortPawn* InstigatorPawn = GetInstigatorFortPlayerPawn();          // 0x14268c4c0
	uint8 PlacementResult = CanPlaceDecoAt(DecoItemDef, AttachedActor, InstigatorPawn, this, Location, Rotation, PlacementSettings);

	if (PlacementResult == 0 || (PlacementResult == 2 && DecoItemDef->bFlag_b70))
	{
		// Direct placement path: virtual 0xb10 -> 0x1426b4070 (195 lines, captured, NOT ported in this pass).
		bool bPlaced = TryPlaceDecoVirtual(Location, Rotation, AttachedActor, InBuildingAttachmentType);
		if (bPlaced)
		{
			// Only trap items (UFortTrapItemDefinition) pass the item through; others pass null.
			UFortDecoItemDefinition* TrapItemDef = DecoItemDef->IsA(UFortTrapItemDefinition::StaticClass()) ? DecoItemDef : nullptr;   // 0x142b218b0
			// virtual 0xac8 -> 0x1426b5370 (254 lines, captured, NOT ported in this pass). Flags = 5, or 6 for trap items.
			AActor* SpawnedDeco = SpawnDecoFromClass(BuildingClass, Location, Rotation, AttachedActor, InBuildingAttachmentType, (TrapItemDef != nullptr) ? 6 : 5);
			if (SpawnedDeco != nullptr)
			{
				// virtual 0xb30 -> 0x1406801c0: Stub_Empty (no-op).
				OnDecoSpawned(SpawnedDeco, Location, InBuildingAttachmentType);

				AFortPlayerController* PC2 = GetInstigatorController();
				UFortDecoItemDefinition* ItemDef2 = GetDecoItemDefinition();
				if (PC2 != nullptr && ItemDef2 != nullptr)
				{
					if (ItemDef2->bFlag_b34 & 1)
					{
						// 0x142a3ee00: PC->ProcessEvent(FName_1481af1a0, { CachedX, CachedY, CachedZ, CachedW }, true)
						PC2->CallEvent_1481af1a0(FVector4(CachedPlacementX, CachedPlacementY_8c4, CachedPlacementZ, CachedPlacementW_8cc), true);
					}
					if ((ItemDef2->bFlag_b34 >> 1) & 1)
					{
						// 0x142a3bde0: PC->ProcessEvent(FName_1481aef78, no params)
						PC2->CallEvent_1481aef78();
					}
				}
			}
		}
	}
}
