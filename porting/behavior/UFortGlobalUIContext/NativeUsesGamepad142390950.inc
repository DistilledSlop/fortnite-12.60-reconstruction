// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
ULocalPlayer* Player = Controller ? Controller->GetLocalPlayer() : nullptr;
UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
return Input && Input->CurrentInputType == ECommonInputType::Gamepad;
