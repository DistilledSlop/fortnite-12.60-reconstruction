// Behaviour snippet: UCreativeHUDLContext::ForwardOnCreativeQuickbarEnabledChanged (body only; spliced into the generated member function).
// Original body: 0x144cc6110 (thunk 0x1451e1540); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV byte [RSP+0x38],DL; ADD RCX,0xe0 -> OnCreativeQuickbarEnabledChanged (+0xe0), one bool param.
// CONFIDENCE: HIGH
OnCreativeQuickbarEnabledChanged.Broadcast(bIsEnabled);
