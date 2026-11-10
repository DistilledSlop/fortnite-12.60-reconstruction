// CONFIDENCE: LOW. Chunk 01 port. See ABuildingGameplayActorPlayerPropAttachment__InitProp.evidence.json for captures and notes.
// Original address: 0x142743f80 (exec). Body 0x141be1620.
if (PropActorToAttach == nullptr)
{
	return;
}

if (PropActorToAttach->IsA(ABuildingActor::StaticClass()))
{
	ABuildingActor* BuildingProp = Cast<ABuildingActor>(PropActorToAttach);
	BuildingProp->bPredictedBuildingActor = false;
	BuildingProp->SetBeingTranslated(true);

	if (BuildingProp->IsA(ABuildingSMActor::StaticClass()))
	{
		// Remove any child building actors still attached to the prop.
		while (BuildingProp->AttachedBuildingActors.Num() > 0 && BuildingProp->AttachedBuildingActors[0] != nullptr)
		{
			BuildingProp->RemoveAttachedBuildingActor(BuildingProp->AttachedBuildingActors[0]);  // vtable slot 0xcb0
		}
	}
}

// Hand the prop to the owning player pawn.
if (AFortPlayerPawn* OwningPawn = Cast<AFortPlayerPawn>(GetOwner()))
{
	OwningPawn->SetPossessedProp(this);
}
