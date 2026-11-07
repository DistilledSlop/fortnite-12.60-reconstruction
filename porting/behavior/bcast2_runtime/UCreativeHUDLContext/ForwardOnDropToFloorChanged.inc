// Behaviour snippet: UCreativeHUDLContext::ForwardOnDropToFloorChanged (body only; spliced into the generated member function).
// Original body: 0x144cc61e0 (thunk 0x1451e1660); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RSP+0x38],DL; ADD RCX,0x70 -> OnDropToFloorChanged (+0x70), FOnMoveToolDropToFloorChanged(bool).
// CONFIDENCE: HIGH
OnDropToFloorChanged.Broadcast(bDropToFloorOn);
