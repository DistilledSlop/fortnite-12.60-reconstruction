// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
APlayerController* Controller = NativeGetContextPlayerController144cb7c20(false);
UFortGlobals* Globals = UFortGlobals::NativeGetGlobals();
const UFortInputData* Data = NativeGetInputData141fc0b30();
if (!Data)
    return NAME_None;
if (Controller && NativeUsesGamepad142390950(Controller) && UFortGameInstance::NativeIsGameMode141ed8de0(Globals, 1))
    return Data->GamepadJumpName;
return Data->JumpName;
