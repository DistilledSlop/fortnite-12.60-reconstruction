// Original address: 0x141601360.
// CONFIDENCE: HIGH
// UFortAbilitySystemGlobals::InitAttributeDefaults, 0x141601360.
// Requires FortAttributeDefaultsPolicy.h; stock FSoftObjectPath stores the
// original 24-byte element's leading FName as its asset path name.
if (FortAttributeDefaultsPolicy::IsBROnly())
{
    GlobalAttributeSetDefaultsTableNames.RemoveAll([this](const FSoftObjectPath& TableName)
    {
        return BROnlyAttributeSetDefaultsToExclude.Contains(TableName.GetAssetPathName());
    });
}
Super::InitAttributeDefaults();
