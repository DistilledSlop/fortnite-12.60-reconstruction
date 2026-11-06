// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
const UFortInputData* Data = NativeGetInputData141fc0b30();
if (!Data)
    return NAME_None;
if (PickerMode == EFortPickerMode::DirectPickEmote)
{
    (void)UFortGlobals::NativeGetGlobals();
    if (UFortGameInstance::NativeGetGameMode141ed4e30(this) == 0)
        return Data->PickerCancelName;
    ULocalPlayer* Player = Cast<ULocalPlayer>(GetOuter());
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    return Input && Input->CurrentInputType == ECommonInputType::Gamepad ? Data->GamepadEmotePickerOrReplayLastEmoteHoldName : Data->EmotePickerName;
}
if (PickerMode == EFortPickerMode::SquadQuickChat)
{
    ULocalPlayer* Player = Cast<ULocalPlayer>(GetOuter());
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    return Input && Input->CurrentInputType == ECommonInputType::Gamepad ? Data->GamepadSquadQuickChatPickerName : Data->SquadQuickChatPickerName;
}
return Data->PickerCancelName;
