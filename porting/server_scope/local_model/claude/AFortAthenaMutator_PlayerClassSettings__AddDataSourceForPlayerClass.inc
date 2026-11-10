// CONFIDENCE: MEDIUM. Original address: 0x142814e10 (exec thunk). Helper: 0x141af3ea0 (captured, read in full).
// Behaviour: register DataSource under ClassSlotIndex. Only objects implementing a property-override or inventory-override
// provider interface are accepted. A new source is added to the source->slot map; an existing one is moved to the new slot.
// The slot's DataSources list is then updated with the source (duplicates are not re-added).
if (DataSource == nullptr)
{
    return false;
}

UClass* SourceClass = DataSource->GetClass();
if (SourceClass == nullptr)
{
    return false;
}
const bool bImplementsProvider =
    SourceClass->ImplementsInterface(UFortPropertyOverrideProvider::StaticClass()) ||
    SourceClass->ImplementsInterface(UFortInventoryOverrideProvider::StaticClass());
if (!bImplementsProvider)
{
    return false;
}

const FWeakObjectPtr SourceKey(DataSource);
uint8* ExistingSlot = DataSourceClassSlots.Find(SourceKey);
if (ExistingSlot == nullptr)
{
    DataSourceClassSlots.Add(SourceKey, ClassSlotIndex);
}
else
{
    // Move the source out of its previous slot list before re-homing it.
    if (FClassSettingsData* OldSlot = ClassSettingsDataMap.Find(*ExistingSlot))
    {
        OldSlot->DataSources.RemoveAll([&SourceKey](const FWeakObjectPtr& Entry) { return Entry == SourceKey; });
    }
    *ExistingSlot = ClassSlotIndex;
}

// Add the source to the destination slot list unless it is already present.
FClassSettingsData& DestSlot = ClassSettingsDataMap.FindOrAdd(ClassSlotIndex);
if (!DestSlot.DataSources.Contains(SourceKey))
{
    DestSlot.DataSources.Add(SourceKey);
}
return true;
