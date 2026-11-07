// Original body 0x142665450 (thunk 0x1428c21a0). Offset evidence: TEST RDX: FortPC null check. MOV RCX,[RDX+0x22f0]: AFortPlayerControllerGameplay.CreativeQuickbarComponent (exact). TEST RCX: null check. ADD RCX,0xe0 -> OnItemAddedToMoveTool; XOR EDX: no params.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
if (FortPC != nullptr && FortPC->CreativeQuickbarComponent != nullptr)
{
    FortPC->CreativeQuickbarComponent->OnItemAddedToMoveTool.Broadcast();
}
