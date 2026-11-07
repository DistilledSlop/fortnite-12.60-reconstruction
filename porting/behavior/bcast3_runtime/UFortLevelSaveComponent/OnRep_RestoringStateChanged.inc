// Original body 0x141eff490 (thunk 0x14297f370). Offset evidence: MOVZX EAX,[RCX+0x3b3]: RestoringState (reflected exact, EBackupSaveState). ADD RCX,0x1b0 -> OnClientBackupRestoreStateChanged (exact). Pack: [0]=AL.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: MEDIUM
OnClientBackupRestoreStateChanged.Broadcast(RestoringState);
