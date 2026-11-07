// Behaviour snippet: AFortPlayerStateZone::OnRep_SpectatingTarget (body only; spliced into the generated member function).
// Original body: 0x142420c20 (thunk 0x14290da10); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV RAX,qword [RCX+0x9e0] (SpectatingTarget, reflected Object) stored in the params block; ADD RCX,0xb38 -> OnSpectatingTargetChangedEvent (+0xb38), FOnSpectatingTargetChanged(AFortPlayerStateZone*).
// CONFIDENCE: HIGH
OnSpectatingTargetChangedEvent.Broadcast(SpectatingTarget);
