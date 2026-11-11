// CONFIDENCE: MEDIUM. Original exec 0x14296dd10; body 0x141ef3c30. Same walk as GetHeldObjectAttachedToPlayer, but returns the UFortHeldObjectComponent itself.
// Returns null when the pawn is null or no attached actor carries the component.
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

	if (UFortHeldObjectComponent* HeldComponent = Cast<UFortHeldObjectComponent>(AttachedActor->GetComponentByClass(UFortHeldObjectComponent::StaticClass())))
	{
		return HeldComponent;
	}
}

return nullptr;
