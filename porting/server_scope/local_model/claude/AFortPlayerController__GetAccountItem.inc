// CONFIDENCE: MEDIUM - the lookup helper 0x1410fe060 forwards to an unnamed virtual on the profile sub-object (+0xd8, slot 0x38); its arguments are not recoverable from the tail call.
// Original address: 0x142a46e80 (exec thunk). Helper: 0x14237ca50. Lookup helper: 0x1410fe060.
// Returns the account item stored on the record found in MainMcpProfile for Id, but only when that item is a UFortAccountItem.
if (MainMcpProfile == nullptr)
{
	return nullptr;
}
UObject* Record = MainMcpProfile->FindAccountRecordForId_0x1410fe060(Id);
if (Record == nullptr)
{
	return nullptr;
}
UObject* Item = Record->AccountItem_0x88;
if (Item != nullptr && Item->IsA(UFortAccountItem::StaticClass()))
{
	return Item;
}
return nullptr;
