// Behaviour snippet: UFortCollectionBookSectionPanel::OnSlotButtonHovered (wrapper, body only).
// Original body address: 0x1450c4fc0 (thunk 0x145210520), this_delta_total 0.
// Offset evidence: CMP byte [this+0x288],0 reads an inherited byte: no reflected member of UFortCollectionBookSectionPanel covers 0x288 (its own members start at 0x420), and it lies in parent storage after UCommonActivatableWidget.BP_OnWidgetDeactivated (0x278), so raw access is used.
// Offset evidence: MOV RCX,[RCX+0x438] loads SlotViewWidget (reflected Object, exact offset 0x438; declared UFortCollectionBookSlotView* in FortniteUI/Generated/UFortCollectionBookSectionPanel.h).
// Offset evidence: MOV RCX,[RCX+0x288] on the slot view loads CollectionBookSlotButtonGroup (declared UCommonButtonGroup* at 0x288 in FortniteUI/Generated/UFortCollectionBookSlotView.h).
// Callee: 0x1407f39f0 -> UCommonButtonGroup::SelectButtonAtIndex(int32 ButtonIndex) (CommonUI engine declaration, CommonButtonGroup.h); RDX = ButtonIx passed through unchanged; JMP is a tail call.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
if (Raw[0x288] != 0)
{
    if (SlotViewWidget != nullptr)
    {
        SlotViewWidget->CollectionBookSlotButtonGroup->SelectButtonAtIndex(ButtonIx);
    }
}
