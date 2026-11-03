// CONFIDENCE: high
const auto GetDisplayTierText = [](EFortDisplayTier Tier) -> FText
{
switch (Tier)
{
case EFortDisplayTier::Invalid: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Invalid", "Invalid");
case EFortDisplayTier::Handmade: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Handmade", "Handmade");
case EFortDisplayTier::Copper: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Copper", "Copper");
case EFortDisplayTier::Silver: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Silver", "Silver");
case EFortDisplayTier::Malachite: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Malachite", "Malachite");
case EFortDisplayTier::Obsidian: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Obsidian", "Obsidian");
case EFortDisplayTier::Brightcore: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Brightcore", "Brightcore");
case EFortDisplayTier::Spectrolite: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Spectrolite", "Spectrolite");
case EFortDisplayTier::Shadowshard: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Shadowshard", "Shadowshard");
case EFortDisplayTier::Sunbeam: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Sunbeam", "Sunbeam");
case EFortDisplayTier::Moonglow: return NSLOCTEXT("Fort.Weapon.Defaults", "EFortDisplayTier.Moonglow", "Moonglow");
default: return FText::GetEmpty();
}
};
if (!bBaseName && !DisplayNamePrefix.IsEmpty())
{
    const FText Name = DisplayName;
    const FText Prefix = DisplayNamePrefix;
    if (DisplayTier != EFortDisplayTier::Invalid)
    {
        const FText Tier = GetDisplayTierText(DisplayTier);
        return FText::Format(NSLOCTEXT("DisplayNameFormat", "DisplayNameFormat", "{0} {1} {2}"), Prefix, Tier, Name);
    }
    return FText::Format(NSLOCTEXT("DisplayNameFormat", "DisplayNameFormatWithoutRarityPrefix", "{0} {1}"), Prefix, Name);
}
if (DisplayTier == EFortDisplayTier::Invalid)
{
    return DisplayName;
}
const FText Name = DisplayName;
const FText Tier = GetDisplayTierText(DisplayTier);
return FText::Format(NSLOCTEXT("DisplayNameFormat", "DisplayNameFormatWithoutPrefix", "{0} {1}"), Tier, Name);
