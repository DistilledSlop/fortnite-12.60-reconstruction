// CONFIDENCE: HIGH. Original exec 0x142a4b060 (RPC thunk, no parameters).
// Validate is virtual slot 0x1300 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x1308 -> 0x1423ab030, which calls virtual slot 0x1638 -> 0x1423786d0 (40 lines). That helper is inlined below.
AActor* CarriedObject = CarriedObject_0x220;
if (CarriedObject != nullptr && CarriedObject->IsA(AFortCarriedObject_Base::StaticClass_0x142a92a00()))
{
	UObject* DropTarget = CarriedObject->GetDropTarget_Slot0x810();
	if (DropTarget != nullptr)
	{
		if (GetLocalRoleByte() < ROLE_Authority)
		{
			DropCarriedObjectRemote_Slot0x1630();
		}
		else
		{
			DropTarget->Sub_0x218->ReleaseCarried_Slot0x18();
		}
	}
}
// Second block: only on the authority with a valid sub-object at +0x248; it calls FUN_141ef3c30 and then Stub_Empty (a no-op).
if (GetLocalRoleByte() == ROLE_Authority && Sub_0x248 != nullptr)
{
	if (Sub_0x248->IsA(AFortGameStateZone_Helper::StaticClass_0x142a75f10()))
	{
		UObject* Helper = FindHelper_0x141ef3c30(Sub_0x248);
		if (Helper != nullptr)
		{
			// Stub_Empty: no effect.
		}
	}
}
