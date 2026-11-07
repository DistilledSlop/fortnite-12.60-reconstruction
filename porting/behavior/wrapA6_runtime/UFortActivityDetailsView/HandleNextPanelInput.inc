// Behaviour snippet: UFortActivityDetailsView::HandleNextPanelInput (wrapper, body only).
// Original body address: 0x144b008b0 (thunk 0x1451f2f10), this_delta_total 0.
// Offset evidence: MOV RCX,[RCX+0x4e0] loads ButtonGroup_PanelTabButtons (reflected Object, exact offset 0x4e0; declared UCommonButtonGroup* in FortniteUI/Generated/UFortActivityDetailsView.h).
// Callee: 0x1407f3a80 -> UCommonButtonGroup::SelectNextButton(bool bAllowWrap) (CommonUI engine declaration, CommonButtonGroup.h); XOR EDX,EDX passes bAllowWrap=false; JMP is a tail call.
// bPassthrough (bool&) is neither read nor written by the body.
// CONFIDENCE: MEDIUM
ButtonGroup_PanelTabButtons->SelectNextButton(false);
