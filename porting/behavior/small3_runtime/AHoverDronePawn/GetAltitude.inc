// Original body: 0x142468970 (thunk 0x142b67360)
// Offset evidence: MOV RAX,[RCX+0x350]; null -> XORPS; else MOVSS XMM0,[RAX+0x20c]
// CONFIDENCE: MEDIUM
if (HoverMoveComponent == nullptr)
{
    return 0.0f;
}
return *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(HoverMoveComponent) + 0x20c);
