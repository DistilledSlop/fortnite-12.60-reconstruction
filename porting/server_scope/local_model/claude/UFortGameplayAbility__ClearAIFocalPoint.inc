// CONFIDENCE: MEDIUM Clears the AI focal point on the owning AI controller. Native checks: the actor info is set, the owner weak pointer is valid, and the owner is an AI controller class; then the Fortnite AI controller focus clear is called with the priority flag.
// Original: exec 0x1428f7de0 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f7de0.c)
if (CurrentActorInfo == nullptr)
{
	return;
}
AFortAIController* FortAIController = Cast<AFortAIController>(CurrentActorInfo->OwnerActor.Get());
if (FortAIController != nullptr)
{
	FortAIController->ClearFocus(bUseAttackingPriority);
}
