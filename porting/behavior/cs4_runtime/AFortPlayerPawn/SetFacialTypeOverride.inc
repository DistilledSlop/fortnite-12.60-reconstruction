// Behaviour snippet: AFortPlayerPawn::SetFacialTypeOverride (body only; spliced into the generated member function).
// Original body: 0x14234c9d0 (thunk 0x142a7fff0).
// Evidence: [this+0x26b4] = FacialTypeOverride (reflected Enum, byte); [this+0x2728] unreflected byte in own region (NativeRecoveredByte_0x2728); tail JMP to static NativeSyncFacialInputToAnimInstances142345bd0(this).
// CONFIDENCE: HIGH
FacialTypeOverride = NewType;
NativeRecoveredByte_0x2728 = static_cast<uint8>(NewType);
NativeSyncFacialInputToAnimInstances142345bd0(this);
