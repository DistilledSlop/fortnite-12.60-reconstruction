// Behaviour snippet: UFortLevelSaveComponent::OnRep_LastCheckpointDate (body only; spliced into the generated member function).
// Original body: 0x141efd9d0 (thunk 0x14297f270); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV RAX,qword [RCX+0x3b8] (LastCheckpointDateUtc, reflected FDateTime) copied to the params block.
//   ADD RCX,0x1c0 -> OnClientCheckpointDateChanged (+0x1c0), FOnDateChanged(FDateTime DateValue).
// CONFIDENCE: HIGH
OnClientCheckpointDateChanged.Broadcast(LastCheckpointDateUtc);
