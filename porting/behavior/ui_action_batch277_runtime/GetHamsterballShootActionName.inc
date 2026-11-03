// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
APlayerController* Controller = NativeGetContextPlayerController144cb7c20(false);
(void)UFortGlobals::NativeGetGlobals();
const UFortInputData* Data = NativeGetInputData141fc0b30();
if (!Data)
    return NAME_None;
return ((NativeUsesGamepad142390950(Controller) || OverrideInputType == ECommonInputType::Gamepad)) ? Data->GamepadHamsterballShootName : Data->HamsterballShootName;
