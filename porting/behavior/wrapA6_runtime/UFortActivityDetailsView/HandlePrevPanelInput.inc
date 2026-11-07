// Behaviour snippet: UFortActivityDetailsView::HandlePrevPanelInput (wrapper, body only).
// Original body address: 0x144b01570 (thunk 0x1451f3060), this_delta_total 0.
// Offset evidence: MOV RCX,[RCX+0x4e0] loads ButtonGroup_PanelTabButtons (reflected Object, exact offset 0x4e0; declared UCommonButtonGroup* in FortniteUI/Generated/UFortActivityDetailsView.h).
// Callee: 0x1407f3ba0 -> UCommonButtonGroup::SelectPreviousButton(bool bAllowWrap) (CommonUI engine declaration, CommonButtonGroup.h); XOR EDX,EDX passes bAllowWrap=false; JMP is a tail call.
// bPassthrough (bool&) is neither read nor written by the body.
// CONFIDENCE: MEDIUM
ButtonGroup_PanelTabButtons->SelectPreviousButton(false);
