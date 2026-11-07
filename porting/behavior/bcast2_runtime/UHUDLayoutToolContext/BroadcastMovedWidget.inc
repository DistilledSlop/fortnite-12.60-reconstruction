// Behaviour snippet: UHUDLayoutToolContext::BroadcastMovedWidget (body only; spliced into the generated member function).
// Original body: 0x144d2d820 (thunk 0x14532ac10); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: TEST RDX,RDX; JZ 0x144d2d83f (epilogue inside body) skips the call when MovedWidget is null.
//   ADD RCX,0xc8 -> OnWidgetMovedDelegate (+0xc8), FOnWidgetMoved(UHUDLayoutToolPlacementWidget*).
// CONFIDENCE: HIGH
if (MovedWidget != nullptr)
{
    OnWidgetMovedDelegate.Broadcast(MovedWidget);
}
