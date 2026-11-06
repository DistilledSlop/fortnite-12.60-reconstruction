// CONFIDENCE: high
// Native helper 0x141f36080 maps ItemType to its template EFortItemType category.
switch (ItemType)
{
case EFortItemType::Badge:
case EFortItemType::BackpackPickup:
case EFortItemType::ItemCache:
case EFortItemType::MissionItem:
case EFortItemType::WorldResource:
    return EFortItemType::WorldItem;
case EFortItemType::BuildingPiece:
case EFortItemType::EditTool:
case EFortItemType::WeaponMelee:
case EFortItemType::WeaponRanged:
case EFortItemType::WeaponHarvest:
case EFortItemType::WeaponCreativePhone:
    return EFortItemType::Weapon;
case EFortItemType::HomebaseGadget:
case EFortItemType::HeroAbility:
    return EFortItemType::Gadget;
case EFortItemType::Stack:
    return EFortItemType::Max_None;
default:
    return ItemType;
}
