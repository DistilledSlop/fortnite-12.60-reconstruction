// Behaviour snippet: UCreativeHUDLContext::ForwardOnCreativeGhostChanged (body only; spliced into the generated member function).
// Original body: 0x144cc6090 (thunk 0x1451e1300); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RSP+0x38],DL; ADD RCX,0x50 -> OnCreativeGhostChanged (+0x50), FOnCreativeGhostChanged(bool bIsGhosting).
// CONFIDENCE: HIGH
OnCreativeGhostChanged.Broadcast(bIsGhostEnabled);
