// CONFIDENCE: high
// Native 0x144d03d50 -> 0x14447f220 -> 0x14237e580 -> 0x141e32a40.
const ULocalPlayer* OuterPlayer = Cast<ULocalPlayer>(GetOuter());
if (!OuterPlayer) return nullptr;
APlayerController* Controller = OuterPlayer->GetPlayerController(GetWorld());
AFortPlayerController* FortController = Cast<AFortPlayerController>(Controller);
if (!FortController) return nullptr;
UFortLocalPlayer* Player = Cast<UFortLocalPlayer>(FortController->Player);
if (!Player || !Player->ClientSettingsRecord) return nullptr;
UFortClientSettingsRecord* Record = Player->ClientSettingsRecord;
return (Record->CloudFileState == ECloudFileState::Saving || Record->CloudFileState == ECloudFileState::Idle) ? Record : nullptr;
