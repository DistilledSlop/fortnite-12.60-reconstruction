// Behaviour snippet: UFortItemManagementScreen::HasItemBeenSeen (wrapper, body only).
// Original body address: 0x144f64810 (thunk 0x145274920), this_delta_total 0.
// Asm: TEST RDX,RDX; JZ to XOR AL,AL (+0x8); MOV RCX,RDX; JMP 0x142014be0 (tail call); XOR AL,AL; RET.
// Offset evidence: no memory access in the body; RDX is the AccountItem argument (UFortAccountItem*), used as the receiver.
// Callee 0x142014be0 -> UFortAccountItem::HasBeenSeenLocally() const (declared in FortniteGame/Generated/UFortAccountItem.h), returns its result.
// Null AccountItem returns false.
// CONFIDENCE: HIGH
if (AccountItem == nullptr)
{
    return false;
}
return AccountItem->HasBeenSeenLocally();
