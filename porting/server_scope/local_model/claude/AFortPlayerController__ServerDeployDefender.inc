// CONFIDENCE: MEDIUM. Original exec 0x142a4ac00 (RPC thunk). Parameters in read order: DefenderTrap (object), SelectedItemInstanceId (FString), DefenderSquadId (FName), WeaponToGiveGuid (FGuid), AmmoQuantity (int32).
// Validate is virtual slot 0x1148 -> 0x1423aaeb0: returns AmmoQuantity >= 0 (the sixth argument is read from the stack). Ported below.
// Implementation is virtual slot 0x1150 -> 0x1423aad80 (57 lines), inlined below.
// Unresolved: the first argument of the item lookup 0x14237ca50 is dropped by the decompile (ported from the impl's use of the result); the trap-component and inventory slot names are placeholders.
if (!(AmmoQuantity >= 0))
{
	RPC_ValidateFailed(TEXT("ServerDeployDefender_Validate"));
	return;
}
if (DefenderTrap == nullptr)
{
	return;
}
FString ItemInstanceId = SelectedItemInstanceId;
UFortAccountItem* DefenderItem = FindAccountItemById(this, ItemInstanceId);
if (DefenderItem == nullptr || !DefenderItem->IsA(UFortDefenderItem::StaticClass()))
{
	return;
}
UObject* WeaponItem = nullptr;
const bool bHasWeaponGuid = WeaponToGiveGuid.A != 0 || WeaponToGiveGuid.B != 0 || WeaponToGiveGuid.C != 0 || WeaponToGiveGuid.D != 0;
if (bHasWeaponGuid)
{
	if (UObject* TrapComponent = DefenderTrap->TrapComponent_0xd10)
	{
		if (UObject* GuidLookup = TrapComponent->GuidLookup_0x1058)
		{
			if (UObject* Inventory = GuidLookup->GetInventory_Slot0x38())
			{
				WeaponItem = Inventory->Sub_0x218->FindByGuid_Slot0x20(WeaponToGiveGuid);
			}
		}
	}
	if (WeaponItem == nullptr)
	{
		WeaponItem = InventoryManager_0x1738->Sub_0x218->FindItemInstanceByGuid_Slot0x18(WeaponToGiveGuid);
	}
}
// Deployment dispatch (0x142760820): ProcessEvent on the trap with the deploy function, parameters {controller, item, squad, weapon, ammo}.
DefenderTrap->DispatchDefenderDeploy(this, DefenderItem, DefenderSquadId, WeaponItem, AmmoQuantity);
