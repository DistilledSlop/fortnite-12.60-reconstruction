// Behaviour snippet: UFortDropdownMenu::GetSelectedButton (body only; spliced into the generated member function).
// Original address: 0x144d7b670 (thunk 0x14522e9d0)
// Evidence: ButtonGroup (+0x438, UCommonButtonGroup*) null -> null. Otherwise SelectedIndex = ButtonGroup->GetSelectedButtonIndex() (0x1407c48f0, const). ButtonGroup is re-read (+0x438), null -> null, then the body tail-jumps to UFortCreativeServersView::NativeGetServerEntry1407c1f20(ButtonGroup, SelectedIndex) (0x1407c1f20), whose result is returned unchanged.
// CONFIDENCE: HIGH
static_assert(sizeof(UCommonButtonGroup) > 0, "Generated header include for UCommonButtonGroup.");
static_assert(sizeof(UFortCreativeServersView) > 0, "Generated header include for UFortCreativeServersView.");
if (ButtonGroup == nullptr)
{
    return nullptr;
}
const int32 SelectedIndex = ButtonGroup->GetSelectedButtonIndex();
if (ButtonGroup == nullptr)
{
    return nullptr;
}
return reinterpret_cast<UCommonButton*>(UFortCreativeServersView::NativeGetServerEntry1407c1f20(ButtonGroup, SelectedIndex));
