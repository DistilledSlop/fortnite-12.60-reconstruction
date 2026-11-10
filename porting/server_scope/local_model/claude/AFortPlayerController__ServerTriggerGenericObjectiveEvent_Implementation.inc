// CONFIDENCE: MEDIUM
// Original: 0x1423ad140 (virtual Implementation of ServerTriggerGenericObjectiveEvent). Callees not named here are described in the evidence file.
// Fire the generic objective event on the objective object; does nothing for a null objective.
if (!Objective)
{
	return;
}
Objective->ProcessEvent(Objective->FindFunctionChecked(NativeGenericObjectiveEventName), nullptr);
