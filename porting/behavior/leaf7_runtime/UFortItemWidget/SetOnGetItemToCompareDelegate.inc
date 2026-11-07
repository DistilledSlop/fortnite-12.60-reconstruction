// Behaviour snippet: UFortItemWidget::SetOnGetItemToCompareDelegate (body only; spliced into the generated member function).
// Original body address: 0x144d96e60. Thunk: 0x14527d690.
// Evidence (asm): MOV RAX,[RDX]; MOV [RCX+0x800],RAX; MOV RAX,[RDX+8]; MOV [RCX+0x808],RAX. A 16-byte copy of the delegate.
// The reflected member OnGetItemToCompareDelegate at +0x800 is skipped (TODO_DelegateNotEmittable_145279f00), so the copy is raw.
// Delegate type not named; a single-cast delegate is 16 bytes (FWeakObjectPtr + FName), so FScriptDelegate is used as the byte-layout stand-in.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
FMemory::Memcpy(Raw + 0x800, &InDelegate, 16);
