// CONFIDENCE: MEDIUM. Original exec 0x14296dd90; body 0x141ef3cf0. Reads the held object's UFortHeldObjectComponent; when its state is not Unheld and it has an owning pawn, returns that pawn.
// The native re-fetches the same component for the final read. Only one lookup is needed here.
if (HeldObject == nullptr)
{
	return nullptr;
}

UFortHeldObjectComponent* HeldComponent = Cast<UFortHeldObjectComponent>(HeldObject->GetComponentByClass(UFortHeldObjectComponent::StaticClass()));
if (HeldComponent == nullptr || HeldComponent->HeldObjectState == EHeldObjectState::Unheld)
{
	return nullptr;
}

return Cast<AFortPlayerPawn>(HeldComponent->OwningPawn.Get());
