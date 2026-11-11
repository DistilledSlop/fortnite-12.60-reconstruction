// CONFIDENCE: MEDIUM. Original exec 0x14296dc90; body 0x141ef3b70. Walks the pawn's attached actors and takes the first one carrying a UFortHeldObjectComponent; returns that component's held object (component field at +0xa0, unreflected).
// Returns null when the pawn is null or no attached actor holds an object.
if (FortPawn == nullptr)
{
	return nullptr;
}

TArray<AActor*> AttachedActors;
FortPawn->GetAttachedActors(AttachedActors);

for (AActor* AttachedActor : AttachedActors)
{
	if (AttachedActor == nullptr)
	{
		continue;
	}

	UFortHeldObjectComponent* HeldComponent = Cast<UFortHeldObjectComponent>(AttachedActor->GetComponentByClass(UFortHeldObjectComponent::StaticClass()));
	if (HeldComponent != nullptr)
	{
		return HeldComponent->HeldObject;
	}
}

return nullptr;
