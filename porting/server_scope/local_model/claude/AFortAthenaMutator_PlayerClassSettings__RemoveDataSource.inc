// CONFIDENCE: MEDIUM. Original address: 0x1428151e0 (exec thunk). Helper: 0x141b0ad20 (captured, read in full).
// Behaviour: remove DataSource from its current slot list and from the source->slot map. Returns false when the source is
// unknown (null, or not in the map).
if (DataSource == nullptr)
{
    return false;
}

const FWeakObjectPtr SourceKey(DataSource);
uint8* SourceSlot = DataSourceClassSlots.Find(SourceKey);
if (SourceSlot == nullptr)
{
    return false;
}

if (FClassSettingsData* Slot = ClassSettingsDataMap.Find(*SourceSlot))
{
    Slot->DataSources.RemoveAll([&SourceKey](const FWeakObjectPtr& Entry) { return Entry == SourceKey; });
}
DataSourceClassSlots.Remove(SourceKey);
return true;
