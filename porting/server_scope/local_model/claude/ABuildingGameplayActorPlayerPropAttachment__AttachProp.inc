// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActorPlayerPropAttachment__AttachProp.evidence.json for captures and notes.
// Original address: 0x142743b30 (exec). Body 0x141bd56a0.
if (PropActorToAttach == nullptr)
{
	return;
}

// Snap the prop onto this actor's root component.
PropActorToAttach->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true), NAME_None);

// When the owner is a player pawn, make this prop its possessed prop.
if (AFortPlayerPawn* OwningPawn = Cast<AFortPlayerPawn>(GetOwner()))
{
	OwningPawn->SetPossessedProp(this);
}
