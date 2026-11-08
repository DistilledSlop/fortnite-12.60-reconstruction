// Behaviour snippet: UFortKismetLibrary::GetFortPlayerControllerFromActor (body only).
// Original address: 0x141ef3070 (thunk 0x14296d850)
// Offset evidence: branch 1 is Actor->IsA(AFortPlayerController) (0x142a3d9f0). Branch 2 is Actor->IsA(APawn) (0x1447ae380) then APawn::Controller at object +0x250 (reflected, GetController()) checked for AFortPlayerController. Branch 3 is Actor->IsA(APlayerState) (0x1447c8410) then AActor::Owner at object +0xe0 (reflected, GetOwner()) checked for AFortPlayerController. The ClassPrivate/depth compares are UObject::IsA. All 75 asm instructions map to these three branches and the final null return.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerController) > 0, "AFortPlayerController must be complete for StaticClass");
static_assert(sizeof(APawn) > 0, "APawn must be complete for GetController");
static_assert(sizeof(APlayerState) > 0, "APlayerState must be complete for IsA");
if (Actor == nullptr)
{
	return nullptr;
}
if (Actor->IsA(AFortPlayerController::StaticClass()))
{
	return reinterpret_cast<AFortPlayerController*>(Actor);
}
if (Actor->IsA(APawn::StaticClass()))
{
	AController* PawnController = static_cast<APawn*>(Actor)->GetController();
	if (PawnController != nullptr && PawnController->IsA(AFortPlayerController::StaticClass()))
	{
		return reinterpret_cast<AFortPlayerController*>(PawnController);
	}
}
if (Actor->IsA(APlayerState::StaticClass()))
{
	AActor* StateOwner = Actor->GetOwner();
	if (StateOwner != nullptr && StateOwner->IsA(AFortPlayerController::StaticClass()))
	{
		return reinterpret_cast<AFortPlayerController*>(StateOwner);
	}
}
return nullptr;
