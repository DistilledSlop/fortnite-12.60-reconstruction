// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
(void)NativeGetContextPlayerController144cb7c20(false);
(void)UFortGlobals::NativeGetGlobals();
const UFortInputData* Data = NativeGetInputData141fc0b30();
return Data ? Data->HotbarKey5Name : NAME_None;
