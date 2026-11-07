// Original body 0x141ade070 (thunk 0x1427faf40). Offset evidence: MOV RAX,[RCX+0x318]: unreflected gap in own region (uint64 member NativeRecoveredPointer_0x318). JZ/JNZ: if RAX != 0 and DL == 0 then MOV byte [RAX+0x2c],1.
// ADD RCX,0x280 -> OnCrucibleParticipantBestTimesUpdated (reflected exact). XOR EDX: no params.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: MEDIUM
if (NativeRecoveredPointer_0x318 != 0 && static_cast<uint8>(CurrentInputType) == 0)
{
    *reinterpret_cast<uint8*>(NativeRecoveredPointer_0x318 + 0x2c) = 1;
}
OnCrucibleParticipantBestTimesUpdated.Broadcast();
