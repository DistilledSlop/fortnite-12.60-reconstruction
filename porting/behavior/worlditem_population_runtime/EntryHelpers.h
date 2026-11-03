#pragma once

// Source equivalents for 0x142038870 and 0x142038ae0. The original
// accepts the ID by value; FGuid::Parse writes directly into ItemGuid.
inline bool NativeSetItemGuidFromString(FFortItemEntry& Entry, FString ItemId)
{
    return FGuid::Parse(ItemId, Entry.ItemGuid);
}

inline void NativeSetEntryParentInventory(FFortItemEntry& Entry,
                                          AFortInventory* Inventory,
                                          bool bReplicatedCopy)
{
    Entry.ParentInventory = Inventory;
    Entry.bIsReplicatedCopy = bReplicatedCopy;
}
