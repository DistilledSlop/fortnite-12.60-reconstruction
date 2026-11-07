// Original body address: 0x144ebd3f0 (thunk 0x1452fb8e0). Offset evidence: receiver weak pointer is the reflected ContextProvidingObject (UCommonPopupMenu, object +0x41c, TWeakObjectPtr<UObject>). Body calls TWeakObjectPtr::Get (0x142ee5dd0), then an inlined class-tree IsA test against UFortStoreSummaryItemButton::StaticClass() (0x1452f8a70), returning the object on match and null otherwise.
// CONFIDENCE: MEDIUM
UObject* Context = ContextProvidingObject.Get();
if (!Context)
{
	return nullptr;
}
return Cast<UFortStoreSummaryItemButton>(Context);
