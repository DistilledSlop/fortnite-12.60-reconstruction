// Behaviour snippet: UFortMatchmakingContext::SetMatchmakingFlowActive (body only; spliced into the generated member function).
// Original body: 0x1421bb980 (thunk 0x14298eec0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RCX+0x194],DL sets bMatchmakingFlowActive (bitfield bool, byte 0x194, mask 0x1); the whole byte is stored.
//   No other declared bit lives in byte 0x194.
//   Asm: ADD RCX,0x60 -> OnMatchmakingFlowChanged (+0x60), FOnMatchmakingFlowChangeDelegate(bool bIsActive).
// CONFIDENCE: MEDIUM
bMatchmakingFlowActive = InFlow;
OnMatchmakingFlowChanged.Broadcast(InFlow);
