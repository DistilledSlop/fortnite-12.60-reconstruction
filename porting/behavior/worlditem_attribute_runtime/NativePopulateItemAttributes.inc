// CONFIDENCE: HIGH
// 0x142035080. Conversion may partially mutate Entry before returning false.
if (!UMcpProfile::NativeDeserializeAttributes(Attributes, &Entry, FFortItemEntry::StaticStruct()))
{
    return false;
}
Entry.AlterationInstances.Reset(Entry.AlterationDefinitions.Num());
for (const FString& Definition : Entry.AlterationDefinitions)
{
    UObject* Asset = UFortAssetManager::NativeGetFortAssetManager().NativeFindAssetByPrimaryId(Definition);
    if (UFortAlterationItemDefinition* Alteration = Cast<UFortAlterationItemDefinition>(Asset))
    {
        Entry.AlterationInstances.Add(Alteration);
    }
    else
    {
        UE_LOG(LogFort, Warning, TEXT("Failed to find asset for alteration %s"), *Definition);
    }
}
return true;
