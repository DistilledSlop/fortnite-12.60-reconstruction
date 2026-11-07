// Original body address: 0x144cf6d60 (thunk 0x14525dab0)
// Offset evidence: same Items loop and inline class-tree test as AreAnyItemsAnimate at 0x144cf6ce0; the JG (depth) and JNZ (ancestor mismatch) exits both jump to the return-true block, so a non-null item failing the test returns true; null items are skipped; else false.
// CONFIDENCE: MEDIUM
for (UFortItem* Item : Items)
{
    if (Item != nullptr && !Item->IsA(UFortCharacter::StaticClass()))
    {
        return true;
    }
}
return false;
