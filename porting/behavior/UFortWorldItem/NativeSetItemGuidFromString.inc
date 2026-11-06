// CONFIDENCE: HIGH
// 0x142038870. Parse writes directly into the existing GUID, even on failure.
return FGuid::Parse(ItemId, Entry.ItemGuid);
