// Original body: 0x1425116b0 (thunk 0x142858170). Offset evidence: reflected gadgets TArray (Data +0x128, Num +0x130), element stride 0x18 (FFilledGadgetSlot: gadget FString at +0, slot_index int32 at +0x10). The first element with slot_index == SlotIndex is used; if its gadget string has Num > 1 (Len() > 0), NativeFindAssetByPrimaryId(gadget) on NativeGetFortAssetManager() gives the asset, returned only when it IsA UFortGadgetItemDefinition (inline class-tree compare at 0x14251170a-0x142511728 after CALL 0x1428e9db0 = UFortGadgetItemDefinition::StaticClass()).
// CONFIDENCE: MEDIUM
const FFilledGadgetSlot* FoundSlot = nullptr;
for (int32 Index = 0; Index < gadgets.Num(); ++Index)
{
    if (gadgets[Index].slot_index == SlotIndex)
    {
        FoundSlot = &gadgets[Index];
        break;
    }
}
if (FoundSlot == nullptr || FoundSlot->gadget.Len() <= 0)
{
    return nullptr;
}

static_assert(sizeof(UFortAssetManager) > 0, "Asset manager is named for the NativeFindAssetByPrimaryId call");
static_assert(sizeof(UFortGadgetItemDefinition) > 0, "Gadget definition class is named for the IsA check");
UObject* AssetObject = UFortAssetManager::NativeGetFortAssetManager().NativeFindAssetByPrimaryId(FoundSlot->gadget);
if (AssetObject == nullptr)
{
    return nullptr;
}
return Cast<UFortGadgetItemDefinition>(AssetObject);
