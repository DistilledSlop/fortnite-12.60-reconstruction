// Behaviour snippet: AFortGameStateZone::ShouldDisablePlayerTeleportingDuringMissionResults (body only; spliced into the generated member function).
// Original body address: 0x141eb29f0 (thunk 0x14292d5c0)
// Offset evidence: MOVZX EAX,byte [this+0x9e8]; RET. New member NativeRecoveredBool_0x9e8 in the gap after OnCompletionResultEvent (0x9d8) and before TotalPlayerStructures (0x9ec).
// CONFIDENCE: MEDIUM
return NativeRecoveredBool_0x9e8;
