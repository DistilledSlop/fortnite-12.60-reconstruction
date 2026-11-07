// Behaviour snippet: UFortLevelSaveComponent::OnRep_BackupSaveState (wrapper body; spliced into the generated member function).
// Original body address: 0x141efd920 (thunk 0x14297f160, receiver delta 0, this delta total 0).
// Offset evidence: MOVZX EAX,byte [RCX+0x3b2] (reflected BackupSaveState, EBackupSaveState : uint8) stored as the 1-byte param at offset 0; RDX (OldValue) is never read.
// Offset evidence: ADD RCX,0x1a0 -> OnClientBackupSaveStateChanged (reflected at +0x1a0; FOnBackupSaveStateChanged(EBackupSaveState SaveState)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
OnClientBackupSaveStateChanged.Broadcast(BackupSaveState);
