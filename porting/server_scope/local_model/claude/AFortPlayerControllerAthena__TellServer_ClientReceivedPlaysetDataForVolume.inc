// CONFIDENCE: LOW. Original exec 0x142a5d630 (RPC thunk, parameters PlaysetData (struct copy) and Volume (object), in thunk read order).
// Validate is virtual slot 0x2330 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2338 -> 0x1419fd510, inlined below.
// The decompiled implementation labels its parameters in the reverse order to the thunk. This port follows the thunk order and the decompiled data flow: the struct copy is the first argument, the object the second. Treat the naming as unverified.
// Callees: FUN_142b86500 (PlaysetLevelStreamComponent::StaticClass), FUN_143f74cc0 (virtual slot 0x610 on the object: find component, body read), FUN_1424255f0 (RegisteringClientData, first 40 lines read).
if (PlaysetDataArg != nullptr)
{
	UObject* StreamComponent = FindPlaysetLevelStreamComponent(VolumeArg); // FUN_143f74cc0 (virtual slot 0x610)
	if (StreamComponent != nullptr && StreamComponent->IsA(UPlaysetLevelStreamComponent::StaticClass()))
	{
		// Volume identity copied as in the decompile: vtable word, ObjectFlags/InternalIndex, ClassPrivate, NamePrivate (ComparisonIndex, Number), OuterPrivate.
		FVolumeKey Key = MakeVolumeKey(PlaysetDataArg);
		StreamComponent->RegisterClientData(this, Key); // FUN_1424255f0 (logs "RegisteringClientData for: %s")
	}
}
