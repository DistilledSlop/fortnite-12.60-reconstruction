// Behaviour snippet: UFortHUDContext::RequestMgmtMenuTab (body only; spliced into the generated member function).
// Original body: 0x144ce56a0 (thunk 0x145258100); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV qword [RSP+0x30],RDX (FName by value); ADD RCX,0x78 -> OnMgmtMenuTabChangeRequested (+0x78), FOnMgmtMenuTabChangeRequestedDelegate(FName).
// CONFIDENCE: HIGH
OnMgmtMenuTabChangeRequested.Broadcast(TabName);
