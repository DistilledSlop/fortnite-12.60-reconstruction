// Original body address: 0x144cf6ce0 (thunk 0x14525da00)
// Offset evidence: loop over TArray<UFortItem*> Items (Data [RCX], Num dword [RCX+8]); per non-null item an inline class-tree test against UFortCharacter::StaticClass() (item ClassPrivate +0x10, class depth +0x38, ancestor array +0x30); first match returns true, else false.
// CONFIDENCE: MEDIUM
for (UFortItem* Item : Items)
{
    if (Item != nullptr && Item->IsA(UFortCharacter::StaticClass()))
    {
        return true;
    }
}
return false;
