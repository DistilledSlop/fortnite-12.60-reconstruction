// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4df30. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ad140.
// Single object parameter.
if (!ServerTriggerGenericObjectiveEvent_Validate(Objective))
{
	RPC_ValidateFailed(TEXT("ServerTriggerGenericObjectiveEvent_Validate"));
	return;
}
ServerTriggerGenericObjectiveEvent_Implementation(Objective);
