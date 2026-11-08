// Original body 0x1418be770 (thunk 0x1428a5770) for UFortControllerComponent_MapDiscoverability::DiscoverAllAreas.
// Offset evidence: [RCX+0x158] is DiscoveredData (reflected FFastArraySerializer); Entries at +0x108 (=0x260, Num 0x268), stride 0x1c; entry byte +0x14 is the bDiscovered bool (reflection gives no struct bit offset), written raw.
// Dirty sequence (ReplicationID/IDCounter, ReplicationKey++, ItemMap reset 0x14072a990, ArrayReplicationKey++, CachedNumItems = -1) is FFastArraySerializer::MarkItemDirty; byte store happens before the dirty marking.
// CONFIDENCE: MEDIUM
for (FPoiDiscoverabilityDataEntry& Entry : DiscoveredData.Entries)
{
	uint8* EntryBytes = reinterpret_cast<uint8*>(&Entry);
	EntryBytes[0x14] = 1;
	DiscoveredData.MarkItemDirty(Entry);
}
