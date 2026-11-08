// Helper 0x143d5e2e0 (UCommonUILibrary). Asm: MOV RAX,[RCX+0x28]; if null return it; else MOV RAX,[RAX+0x28]; RET.
// Offset evidence: UWidget +0x28 is reflected as Slot (Object); UPanelSlot +0x28 is reflected as Parent (Object). Both used by name.
// Caller evidence: FindParentWidgetOfType @0x1407bec70 loops on the return value as the next parent widget, passing the widget in RCX.
// CONFIDENCE: HIGH

UPanelSlot* ParentSlot = WidgetObj->Slot;
if (ParentSlot == nullptr)
{
    return nullptr;
}
return ParentSlot->Parent;
