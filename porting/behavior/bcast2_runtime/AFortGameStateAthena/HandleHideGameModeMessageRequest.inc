// Behaviour snippet: AFortGameStateAthena::HandleHideGameModeMessageRequest (body only; spliced into the generated member function).
// Original body: 0x14199cab0 (thunk 0x14291fcc0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: [RSP+0x38]=DL (MsgType), [RSP+0x39]=R8B (bInstantHide), [RSP+0x3a]=R9B (bClearQueue).
//   ADD RCX,0x10d8 -> HideGameModeMessageRequest (+0x10d8), FOnHideGameModeMessageRequest(EAthenaGameMsgType, bool, bool) in the same order.
// CONFIDENCE: HIGH
HideGameModeMessageRequest.Broadcast(MsgType, bInstantHide, bClearQueue);
