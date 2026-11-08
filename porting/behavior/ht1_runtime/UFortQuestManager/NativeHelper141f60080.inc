// Helper 0x141f60080 (UFortQuestManager). Asm: ADD RCX,0x1e4 ; JMP 0x142ee5dd0 (TWeakObjectPtr::Get). Whole helper is 2 instructions.
// Offset evidence: [this+0x1e4] is an 8-byte TWeakObjectPtr<UObject>. lookup_offset.py UFortQuestManager 0x1e4 finds no reflected member, so raw access is used.
// Caller evidence: five callers are UFortQuestManager members that pass this in RCX; SlotItem @0x1424bd7f0 passes RDI before CALL 0x141f60080 and returns the result.
// Identity of the referenced object is not established, so the helper keeps a neutral name.
// CONFIDENCE: HIGH

const TWeakObjectPtr<UObject>& WeakRef1e4 = *reinterpret_cast<const TWeakObjectPtr<UObject>*>(reinterpret_cast<const uint8*>(QuestManagerObj) + 0x1e4);
return WeakRef1e4.Get();
