// Original body address: 0x1424443c0 (thunk 0x142ab9830)
// Offset evidence: TEST RDX,RDX; JNZ 0x1424443c8 -> null Quest path is XOR AL,AL; RET (returns false). Non-null path: MOV RCX,RDX; JMP 0x142014be0, so the receiver is the Quest argument and the quest manager this is unused.
// Callee: 0x142014be0 -> UFortAccountItem::HasBeenSeenLocally() const (tail call, bool result); UFortQuestItem derives from UFortAccountItem.
// CONFIDENCE: HIGH
if (Quest == nullptr)
{
    return false;
}
return Quest->HasBeenSeenLocally();
