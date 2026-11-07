// Original body address: 0x1422dca60 (thunk 0x1427a4f50)
// Offset evidence: +0x250 is APawn::Controller (engine storage, read raw); IsA AFortAIController checked as in the asm.
// AFortAIController::GoalActor at +0x368 (reflected); GoalActor byte +0x5c (unreflected pointee, raw) masked 0x60 must equal 0x40.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* OwningController = *reinterpret_cast<UObject* const*>(Raw + 0x250);
if (OwningController == nullptr || !OwningController->IsA(AFortAIController::StaticClass()))
{
	return false;
}
AFortAIController* AIController = static_cast<AFortAIController*>(OwningController);
if (AIController->GoalActor == nullptr)
{
	return false;
}
const uint8* GoalRaw = reinterpret_cast<const uint8*>(AIController->GoalActor);
return (GoalRaw[0x5c] & 0x60) == 0x40;
