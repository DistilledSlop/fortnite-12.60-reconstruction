// Original body 0x1424ee1c0 (thunk 0x142924a90). Offset evidence: CMP qword [RCX+0xfc8],0: DeployableBaseManager (reflected exact). ADD RCX,0xfd0 -> OnDeployableBaseManagerCreated (reflected exact). XOR EDX: no params.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
if (DeployableBaseManager != nullptr)
{
    OnDeployableBaseManagerCreated.Broadcast();
}
