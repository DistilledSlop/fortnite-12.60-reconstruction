// Helper 0x1450b2410 (UFortCollectionBookSectionPanel). Asm evidence:
//   [this+0x288] = UCommonButtonGroup (null -> return 0)
//   GetSelectedButtonIndex(group) == -1 -> return 0
//   NativeGetServerEntry1407c1f20(group, index) -> entry (null -> return 0)
//   entry IsA UFortCollectionBookSlotButton (class-tree check, 0x1450b2448..0x1450b2465) -> entry, else 0
// CONFIDENCE: HIGH
UCommonButtonGroup* SelectedGroup = *reinterpret_cast<UCommonButtonGroup* const*>(reinterpret_cast<const uint8*>(Panel) + 0x288);
if (SelectedGroup == nullptr)
{
    return nullptr;
}
const int32 SelectedIndex = SelectedGroup->GetSelectedButtonIndex();
if (SelectedIndex == -1)
{
    return nullptr;
}
UObject* SelectedEntry = UFortCreativeServersView::NativeGetServerEntry1407c1f20(SelectedGroup, SelectedIndex);
if (SelectedEntry == nullptr)
{
    return nullptr;
}
if (SelectedEntry->IsA(UFortCollectionBookSlotButton::StaticClass()))
{
    return SelectedEntry;
}
return nullptr;
