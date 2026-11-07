// Behaviour snippet: UAthenaHUDContext::ForceVictoryRoyaleUI (body only; spliced into the generated member function).
// Original body: 0x144c8c6f0 (thunk 0x14519d7e0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RCX+0x24a],1 sets bLocalPlayerWonMatch (bitfield bool, byte 0x24a, mask 0x1).
//   The body stores the whole byte; no other declared bit lives in byte 0x24a.
//   Asm: ADD RCX,0x38; XOR EDX,EDX -> OnLocalPlayerWon (+0x38), FOnLocalPlayerWon has no params.
// CONFIDENCE: MEDIUM
bLocalPlayerWonMatch = true;
OnLocalPlayerWon.Broadcast();
