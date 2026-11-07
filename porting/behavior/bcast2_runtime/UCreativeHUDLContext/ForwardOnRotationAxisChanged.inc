// Behaviour snippet: UCreativeHUDLContext::ForwardOnRotationAxisChanged (body only; spliced into the generated member function).
// Original body: 0x1407c9550 (thunk 0x1451e19d0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RSP+0x38],DL; ADD RCX,0x30 -> OnRotationAxisChanged (+0x30), FOnMoveToolRotationAxisChanged(uint8 RotationAxisIndex).
// CONFIDENCE: HIGH
OnRotationAxisChanged.Broadcast(AxisIndex);
