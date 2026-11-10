// CONFIDENCE: LOW. Original exec 0x142a4a8f0 (RPC thunk). Parameters in read order: ItemId (FString), PostCraftSlot (int32), CraftAmount (int32), RequestedTier (byte enum), bIsQuickCrafted (bool).
// Validate is virtual slot 0x10a0 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x10a8 -> 0x1423aa9f0 (149 lines), flow inlined below. Known gaps: the decompile drops the arguments of the item lookup (0x14237ca50) and of the first 0x140682ce0 call, so the lookup key is taken from ItemId by assumption; the global-tag and shared-context reads are unnamed; the STRIPPED call 0x140680300 is skipped (see below).
UFortAccountItem* SchematicItem = FindAccountItemById(this, ItemId);
if (SchematicItem != nullptr && SchematicItem->IsA(UFortSchematicItem::StaticClass()))
{
	const uint32 CraftResult = SchematicItem->Craft_Slot0x518(this, PostCraftSlot, CraftAmount, RequestedTier);
	UWorld* World = GetWorld();
	UFortMissionManager* MissionManager = GetZoneMissionManager_0x1420b7ab0(World);
	if (MissionManager == nullptr)
	{
		return;
	}
	UFortItemCraftedParams* CraftedParams = NewObject<UFortItemCraftedParams>(GetTransientPackage_0x142e80820());
	const UObject* RequestedTierObject = GetRequestedTierObject_0x1425490f0(SchematicItem, RequestedTier);
	CraftedParams->TierObject_0x28 = RequestedTierObject;
	CraftedParams->Controller_0x30 = this;
	CraftedParams->CraftResult_0x38 = CraftResult;
	CraftedParams->bQuickCraft_0x3c = bIsQuickCrafted;

	FFortMissionEventData Data;
	Data.Tag = CraftingEventTag_0x14813fa58;
	Data.Context = GetMissionEventContext_0x1407c2c80();
	Data.Payload = CraftedParams;
	// Dead branch in the decompile: a lookup through the STRIPPED stub 0x140680300 always yields 0, so its follow-up call is never reached.
	MissionManager->HandleMissionEvent_Slot0x638(Data);
}
else
{
	UE_LOG(LogFortItems, Display, TEXT("Cannot get SchematicItem for schematic crafting"));
}
