// CONFIDENCE: MEDIUM - the reads are fully mapped; the class default object is read at UClass offset 0x130, and the force-init virtual (slot 0x390 on UClass) is unnamed.
// Original address: 0x142a49c60 (exec thunk, no parameters). Class is UObject::ClassPrivate (offset 0x10).
// Copies InputPitchScale (0x428) and InputYawScale (0x424) from the class default object of this controller's class.
UClass* Cls = GetClass();
UObject* CDO = Cls->ClassDefaultObject_0x130;
if (CDO == nullptr)
{
	Cls->ForceInitDefaultObject_Slot0x390();
	CDO = Cls->ClassDefaultObject_0x130;
	if (CDO == nullptr)
	{
		return;
	}
}
AFortPlayerController* Defaults = Cast<AFortPlayerController>(CDO);
InputPitchScale = Defaults->InputPitchScale;
InputYawScale = Defaults->InputYawScale;
