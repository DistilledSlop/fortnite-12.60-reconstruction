// Behaviour snippet: UCreativeHUDLContext::ForwardOnMoveToolScaleModified (body only; spliced into the generated member function).
// Original body: 0x144cc66e0 (thunk 0x1451e17f0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOVSD XMM0,[RDX] and MOV EAX,[RDX+8] copy the FVector (12 bytes) into the params block.
//   ADD RCX,0xb0 -> OnMoveToolScaleModified (+0xb0), FOnMoveToolScaleModified(FVector currentScale).
// CONFIDENCE: HIGH
OnMoveToolScaleModified.Broadcast(currentVector);
