// Helper 0x141692dd0 (caller ReleaseBuildTool 0x1416a9020 passes RCX = controller, RDX = PendingEquipWeapon).
// - [this+0x908] raw byte (unreflected, right after FortControllerComponent_Telemetry @0x900): when set the request is deferred
//   by storing it in PendingEquipWeapon (+0x910) and the function returns null.
// - [this+0x400] = PlayerBotPawn (lookup_offset). vtbl[0x3e8] on the item = UFortWorldItem::GetItemDefinitionBP (0x14205a2f0, see
//   UFortWorldItem.cpp); vtbl[0x468] = GetItemGuid (UFortItem base 0x140693ec0 zero-fills the sret FGuid).
// - 0x142b4a5d0/0x1428e9db0 = UFortWeaponItemDefinition/UFortGadgetItemDefinition::StaticClass (inline IsA chain checks = Cast<>);
//   0x1420140e0 = UFortGadgetItemDefinition::GetWeaponItemDefinition; 0x1423150b0 = AFortPawn::EquipWeaponDefinition
//   (reflected params by offset: WeaponData @0, ItemEntryGuid @8, ReturnValue @0x18).
if (*(reinterpret_cast<const uint8*>(BotController) + 0x908) != 0)
{
	BotController->PendingEquipWeapon = ItemToEquip;
	return nullptr;
}
if (ItemToEquip == nullptr || BotController->PlayerBotPawn == nullptr)
{
	return nullptr;
}
UFortWeaponItemDefinition* WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemToEquip->GetItemDefinitionBP());
if (WeaponDefinition == nullptr)
{
	UFortGadgetItemDefinition* GadgetDefinition = Cast<UFortGadgetItemDefinition>(ItemToEquip->GetItemDefinitionBP());
	if (GadgetDefinition == nullptr)
	{
		return nullptr;
	}
	WeaponDefinition = GadgetDefinition->GetWeaponItemDefinition();
}
if (WeaponDefinition == nullptr)
{
	return nullptr;
}
const FGuid EquipItemGuid = ItemToEquip->GetItemGuid();
return BotController->PlayerBotPawn->EquipWeaponDefinition(WeaponDefinition, EquipItemGuid);
// CONFIDENCE: MEDIUM
