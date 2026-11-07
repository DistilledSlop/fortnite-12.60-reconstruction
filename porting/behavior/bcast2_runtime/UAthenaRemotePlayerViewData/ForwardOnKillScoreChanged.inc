// Behaviour snippet: UAthenaRemotePlayerViewData::ForwardOnKillScoreChanged (body only; spliced into the generated member function).
// Original body: 0x144c20040 (thunk 0x1451b8630); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV dword [RCX+0x200],R8D stores InKillScore at object offset 0x200.
//   No reflected member covers 0x200 and the class own region starts at 0x28: member_field NativeRecoveredInt_0x200.
//   Asm: ADD RCX,0x68 -> OnKillScoreChanged (+0x68), FIntDelegate(UAthenaRemotePlayerViewData* PlayerData, int32 IntParam); slot0 is this.
// CONFIDENCE: MEDIUM
NativeRecoveredInt_0x200 = InKillScore;
OnKillScoreChanged.Broadcast(this, InKillScore);
