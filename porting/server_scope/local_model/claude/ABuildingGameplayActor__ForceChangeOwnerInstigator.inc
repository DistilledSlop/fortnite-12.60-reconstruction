// CONFIDENCE: LOW. Chunk 01 port. See ABuildingGameplayActor__ForceChangeOwnerInstigator.evidence.json for captures and notes.
// Original address: 0x14273ffa0 (exec). Body 0x141bdc580.
// Re-points the building's owner and instigator to a new pawn. Authority only; a null pawn does nothing.
if (Role != ROLE_Authority || NewInstigator == nullptr)
{
	return;
}

ResetOwnershipState();                                   // virtual slot 0x440 (AActor)
Instigator = NewInstigator->GetInstigatorReference();    // reads pawn field at +0x248 (name unresolved)
SetOwnerTeamIndex(NewInstigator->GetTeamIndex());        // virtual slot 0x990 (ABuildingActor)
SetOwnerPersistentID(GetPersistentIdForPawn(NewInstigator)); // ABuildingActor::OwnerPersistentID (+0x350)

if (UAbilitySystemComponent* ASC = GetOwnerAbilitySystem())    // this->+0x4e entry vcall 0x10
{
	ASC->NotifyOwnerChanged(this, this);                 // vcall 0x770
}
