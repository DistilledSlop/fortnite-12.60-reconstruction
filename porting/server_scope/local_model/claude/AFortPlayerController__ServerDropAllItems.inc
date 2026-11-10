// CONFIDENCE: MEDIUM. Original exec 0x142a4afa0 (RPC thunk, one object parameter IgnoreItemDef).
// Validate is virtual slot 0xdc8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0xdd0 -> 0x1423aaff0 (15 lines), inlined below. The controller (first implementation argument) is unused.
// Unresolved: the object interface at +0x30 (slot 0x18), the config getter 0x141ed1800 (+0xba8 field), and the interface names.
if (IgnoreItemDef != nullptr)
{
	IIgnoreItemDefInterface* Interface = IgnoreItemDef->GetInterface_0x30();
	Interface->Apply_Slot0x18(ItemOptionsConfig_0x141ed1800()->Field_0xba8);
}
