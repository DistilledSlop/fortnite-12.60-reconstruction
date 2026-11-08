// Helper 0x14237e580, host AFortPlayerControllerGameplay (static). 29 instructions.
// [this+0x290] is APlayerController::Player (reflected). The helper requires it to be a UFortLocalPlayer (IsA via the
//   class-tree test expanded from the StaticClass call 0x142989f60, hint UFortLocalPlayer::StaticClass()).
// [LocalPlayer+0x320] is UFortLocalPlayer::ClientSettingsRecord (reflected). It must be non-null and pass
//   UFortLocalPlayer::NativeHelper141e32a40 (0x141e32a40); the record pointer is then returned.
// The asm forwards the ClientSettingsRecord pointer to a helper declared with a UFortLocalPlayer* parameter; the pointer is passed unchanged.
// Callers: UFortPartyContext::ShouldShowCrossplayWarningDialog (0x142270d90), AFortPlayerControllerGameplay::IsUsingHideOtherPlayerNames (0x1423d97d0).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete");
UPlayer* PlayerObject = PlayerControllerObject->Player;
if (PlayerObject == nullptr)
{
	return nullptr;
}
if (!PlayerObject->IsA(UFortLocalPlayer::StaticClass()))
{
	return nullptr;
}
UFortLocalPlayer* FortLocalPlayer = Cast<UFortLocalPlayer>(PlayerObject);
UObject* SettingsRecord = FortLocalPlayer->ClientSettingsRecord;
if (SettingsRecord == nullptr)
{
	return nullptr;
}
if (!UFortLocalPlayer::NativeHelper141e32a40(reinterpret_cast<UFortLocalPlayer*>(SettingsRecord)))
{
	return nullptr;
}
return FortLocalPlayer->ClientSettingsRecord;
