// CONFIDENCE: MEDIUM. Original exec 0x142a4ba70 (RPC thunk). Parameters in read order: DestroyedItemGuid (FGuid), Count (int32).
// Validate is virtual slot 0x10b8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x10c0 -> 0x1423aba50 (17 lines), inlined below. It packs the guid and count with three false/true/false flags into a parameter block and calls ProcessEvent on this controller with the event function (FName global 0x1481af200).
FFortItemWillBeDestroyedParams Params;
Params.DestroyedItemGuid = DestroyedItemGuid;
Params.Count = Count;
Params.bFlag0 = false;
Params.bFlag1 = true;
Params.bFlag2 = false;
ProcessEvent(FindFunctionChecked(ItemWillBeDestroyedEventName_0x1481af200), &Params);
