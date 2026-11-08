// Original body: 0x142483df0 (thunk 0x142b90540). Offset evidence: [RCX+0x278] is the existing member NativeRecoveredUint64_0x278 (object offset 0x278, directly after FString string_d at 0x268). The helper call's result is only consumed by an AFortPlayerControllerAthena type check whose only effect is a call to an empty function (0x1406801c0); both branches change no state, so that tail is omitted.
// CONFIDENCE: MEDIUM
UObject* Context = reinterpret_cast<UObject*>(NativeRecoveredUint64_0x278);
(void)UFortMinigameStatics::NativeHelper142414c60(Context);
