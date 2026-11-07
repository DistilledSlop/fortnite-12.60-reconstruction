// Original body: 0x1424795a0 (thunk 0x142b67780)
// Offset evidence: MOV RAX,[RCX+0x350]; JZ -> RET with AL=0; else MOVZX byte [RAX+0x21d]; AND AL,1
// CONFIDENCE: MEDIUM
if (HoverMoveComponent == nullptr)
{
    return false;
}
return (*(reinterpret_cast<const uint8*>(HoverMoveComponent) + 0x21d) & 0x1) != 0;
