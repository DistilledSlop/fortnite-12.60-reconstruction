// Behaviour snippet: UFortLlamaStoreInspectionScreen::IsItemChoicePack (body only).
// Original body address: 0x1450b8c40 (thunk 0x145285d30). this_delta_total 0; no receiver member is accessed (RCX is passed through to the tail call).
// Offset evidence: RDX = ItemToCheck. The body null-checks it, calls UFortCardPackItem::StaticClass() (0x142859760), and compares ItemToCheck's class ancestry with that class (ClassPrivate at +0x10 and the depth/ancestor table): the inlined UObject::IsA(UFortCardPackItem::StaticClass()). On success it tail-jumps (JMP 0x14254c020) to UFortCardPackItem::IsChoicePack with RCX = ItemToCheck; otherwise returns false (XOR AL,AL).
// CONFIDENCE: HIGH
if (ItemToCheck == nullptr)
{
    return false;
}
if (!ItemToCheck->IsA(UFortCardPackItem::StaticClass()))
{
    return false;
}
return static_cast<UFortCardPackItem*>(ItemToCheck)->IsChoicePack();
