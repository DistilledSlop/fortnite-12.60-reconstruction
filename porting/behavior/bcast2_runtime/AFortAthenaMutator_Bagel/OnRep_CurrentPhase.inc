// Behaviour snippet: AFortAthenaMutator_Bagel::OnRep_CurrentPhase (body only; spliced into the generated member function).
// Original body: 0x141ab4b00 (thunk 0x1427e9e70); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOVZX EAX,byte [RCX+0x12c0] (CurrentPhase, reflected TEnumAsByte); MOV dword [RSP+0x30],EAX.
//   ADD RCX,0x290 -> OnPhaseChanged (+0x290), FOnBagelPhaseChanged(int32 NewPhase).
// CONFIDENCE: MEDIUM
OnPhaseChanged.Broadcast(static_cast<int32>(CurrentPhase));
