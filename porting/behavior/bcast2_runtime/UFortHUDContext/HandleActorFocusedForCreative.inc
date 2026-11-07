// Behaviour snippet: UFortHUDContext::HandleActorFocusedForCreative (body only; spliced into the generated member function).
// Original body: 0x144cd3840 (thunk 0x1452573d0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV qword [RSP+0x30],RDX; ADD RCX,0xc8 -> OnActorFocusedForCreative (+0xc8), FOnActorFocusedForCreative(AActor*).
// CONFIDENCE: HIGH
OnActorFocusedForCreative.Broadcast(NewActor);
