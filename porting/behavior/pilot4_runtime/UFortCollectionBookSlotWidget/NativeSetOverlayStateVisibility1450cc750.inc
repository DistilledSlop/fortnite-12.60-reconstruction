// Helper 0x1450cc750 (71 insns), called from UFortCollectionBookSlotWidget::OnItemDestroyed (0x1450c3cb0) as
// (this, state) with DL = (bool)+1.
// Offset evidence (lookup_offset.py UFortCollectionBookSlotWidget): +0x278 UnslottedOverlayWidget,
// +0x280 ReadyToSlotOverlayWidget, +0x288 UnslottedButReadyOverlayWidget, all UWidget*.
// Each widget gets CALL [vtable+0x270] with DL = 0 (Visible) or 2 (Hidden); slot 0x270 is UWidget::SetVisibility
// (same slot ShowThrobber 0x1451ee8f0 -> 0x144f7c850 uses for ESlateVisibility::Visible on UImage widgets).
// State 0: all hidden. State 1: Unslotted visible, others hidden. State 2: UnslottedButReady visible, others hidden.
// State 3: ReadyToSlot visible, others hidden. Any other value: no change.
// CONFIDENCE: HIGH
auto SetOverlayVisibility = [](UWidget* TargetWidget, ESlateVisibility NewVisibility)
{
    if (TargetWidget)
    {
        TargetWidget->SetVisibility(NewVisibility);
    }
};
UWidget* UnslottedWidget = Widget->UnslottedOverlayWidget;
UWidget* ReadyToSlotWidget = Widget->ReadyToSlotOverlayWidget;
UWidget* UnslottedButReadyWidget = Widget->UnslottedButReadyOverlayWidget;
if (State > 3)
{
    return;
}
SetOverlayVisibility(UnslottedWidget, State == 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
SetOverlayVisibility(ReadyToSlotWidget, State == 3 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
SetOverlayVisibility(UnslottedButReadyWidget, State == 2 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
