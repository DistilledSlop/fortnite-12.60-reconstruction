// Behaviour snippet: AFortPlayerControllerGameplay::OnRep_UIMetricsDisplayIndex (body only; spliced into the generated member function).
// Original body: 0x1423e0620 (thunk 0x142a65db0); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV EAX,dword [RCX+0x25bc] (UIMetricsDisplayIndex, reflected Int); ADD RCX,0x2160 -> OnCreativeUIMetricsDisplayChanged (+0x2160), FOnCreativeUIMetricsDisplayChanged(int32).
// CONFIDENCE: HIGH
OnCreativeUIMetricsDisplayChanged.Broadcast(UIMetricsDisplayIndex);
