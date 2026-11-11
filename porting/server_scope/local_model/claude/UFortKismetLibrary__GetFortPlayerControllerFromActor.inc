// CONFIDENCE: MEDIUM. Original exec 0x14296d850; body 0x141ef3070. Checks the actor itself, then its pawn controller, then a player state's owner.
// The native returns the first FortPlayerController found in that order. The player-state owner link (native +0xe0) is taken as GetOwner().
if (Actor == nullptr)
{
	return nullptr;
}

if (Actor->IsA(AFortPlayerController::StaticClass()))
{
	return Cast<AFortPlayerController>(Actor);
}

if (APawn* Pawn = Cast<APawn>(Actor))
{
	if (AFortPlayerController* PawnController = Cast<AFortPlayerController>(Pawn->GetController()))
	{
		return PawnController;
	}
}

if (APlayerState* PlayerState = Cast<APlayerState>(Actor))
{
	if (AFortPlayerController* OwnerController = Cast<AFortPlayerController>(PlayerState->GetOwner()))
	{
		return OwnerController;
	}
}

return nullptr;
