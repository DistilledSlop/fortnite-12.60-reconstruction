// Behaviour snippet: UFortReplayContext::ToggleViewSettingsVisibility (body only; spliced into the generated member function).
// Original body: 0x1418ce8e0 (thunk 0x142ac7690); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: ADD RCX,0xd0; XOR EDX,EDX (RDX=0, no params); JMP 0x1406a9dc0.
//   Object offset 0xd0 = OnToggleReplayViewSettings (reflected InlineMulticastDelegate at +0xd0).
//   FToggleReplayViewSettings declares no parameters.
// CONFIDENCE: HIGH
OnToggleReplayViewSettings.Broadcast();
