// Behaviour snippet: UFortItemManagementInventoryPanel::HandleFilterTabButtonCreated (wrapper, body only).
// Original body address: 0x144f28950 (thunk 0x14526f450), this_delta_total 0.
// Asm: TEST R8,R8; JZ to RET (+0x10); MOV RDX,[RCX+0x2c0]; MOV RCX,R8; JMP 0x1407f9c70 (tail call).
// Offset evidence: [RCX+0x2c0] = reflected member FilterTabButtonStyle (Class, exact offset 0x2c0, own member of UFortItemManagementInventoryPanel; declared TSubclassOf<UCommonButtonStyle> at FortniteUI/Generated/UFortItemManagementInventoryPanel.h line 46).
// Callee 0x1407f9c70 -> UCommonButton::SetStyle(TSubclassOf<UCommonButtonStyle> InStyle), receiver = TabButton (R8 argument, class UCommonButton).
// Null TabButton skips the call and returns.
// CONFIDENCE: HIGH
if (TabButton == nullptr)
{
    return;
}
TabButton->SetStyle(FilterTabButtonStyle);
