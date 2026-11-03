// Candidate only: helper and default-text equivalence still require proof.
switch (Reason)
{
case EItemEvolutionRestrictionReason::NoEvolutions:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_NoEvolutions", "Item does not have evolutions.");
case EItemEvolutionRestrictionReason::BelowMaximumLevel:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_BelowMaximumLevel", "Item must attain its maximum level before evolving.");
case EItemEvolutionRestrictionReason::VaultOverflow:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_VaultOverflow", "Items in the vault overflow temporarily can not be evolved.");
case EItemEvolutionRestrictionReason::MissingCatalyst:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_MissingCatalyst", "Not yet possible.");
case EItemEvolutionRestrictionReason::MissingCosts:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_MissingCosts", "Required ingredient cost is not met.");
case EItemEvolutionRestrictionReason::NoRarityUpgrade:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_NoRarityUpgrade", "Item does not have a rarity upgrade.");
case EItemEvolutionRestrictionReason::InUseByCrafting:
    return NSLOCTEXT("FortItem", "ItemEvolutionRestrictionReason_InUseByCrafting", "Item is currently in use by crafting.");
default:
    return FText::GetEmpty();
}
