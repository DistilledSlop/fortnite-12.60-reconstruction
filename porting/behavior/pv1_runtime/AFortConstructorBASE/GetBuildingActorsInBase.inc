// Original body 0x1424e1fd0 (thunk 0x14288ba40) for AFortConstructorBASE::GetBuildingActorsInBase.
// Offset evidence: [RCX+0xd60] is an unreflected TSet of actor pointers (passed by address to 0x1415c07f0, no own reflected member there), accessed as a raw offset; sret TArray in RDX.
// GUObjectArray flag test (bit 29 of FUObjectItem flags) is UObject::IsPendingKill(); push-back is TArray::Add.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
TArray<ABuildingSMActor*> BaseActors;
TArray<ABuildingSMActor*> Result;
AFortConstructorBASE::NativeSetToArray1415c07f0(Raw + 0xd60, &BaseActors);
static_assert(sizeof(ABuildingSMActor) > 0, "ABuildingSMActor must be complete");
for (ABuildingSMActor* BuildingActor : BaseActors)
{
	if (!BuildingActor->IsPendingKill())
	{
		Result.Add(BuildingActor);
	}
}
return Result;
