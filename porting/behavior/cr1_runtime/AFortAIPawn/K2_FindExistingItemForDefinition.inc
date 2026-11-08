// Original body address: 0x1422dd300
// Offset evidence: [RDI+0x1a58] = AFortAIPawn::Inventory (reflected). Null ItemDefinition or non-UFortWorldItemDefinition (inlined class-tree IsA check against UFortWorldItemDefinition::StaticClass()) yields a null definition; null inventory or null definition returns nullptr; otherwise tail-jump to NativeFindWorldItemByDefinition(definition, false) (R8 = 0). bInStorageVault (R8) is never read.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortItemDefinition) > 0, "UFortItemDefinition must be complete for IsA");
static_assert(sizeof(UFortWorldItemDefinition) > 0, "UFortWorldItemDefinition must be complete for StaticClass");
static_assert(sizeof(UFortWorldItem) > 0, "UFortWorldItem must be complete for the UFortItem upcast");
static_assert(sizeof(AFortInventory) > 0, "AFortInventory must be complete for NativeFindWorldItemByDefinition");
(void)bInStorageVault;
UFortItemDefinition* RequestedDefinition = ItemDefinition;
if (RequestedDefinition == nullptr || !RequestedDefinition->IsA(UFortWorldItemDefinition::StaticClass()))
{
    RequestedDefinition = nullptr;
}
if (Inventory == nullptr || RequestedDefinition == nullptr)
{
    return nullptr;
}
return Inventory->NativeFindWorldItemByDefinition(RequestedDefinition, false);
