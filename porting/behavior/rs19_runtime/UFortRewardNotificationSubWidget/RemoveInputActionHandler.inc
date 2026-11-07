// Behaviour snippet: UFortRewardNotificationSubWidget::RemoveInputActionHandler (body only).
// Original body address: 0x144e8bc80 (thunk 0x1452c32f0, group B).
// Offset evidence: UCommonActivatablePanel pointer at +0x278 (NativeRecoveredObject_0x278, unreflected); callees are UCommonActivatablePanel::HasInputActionHandler (const) and UCommonActivatablePanel::RemoveInputActionHandler, both taking the FDataTableRowHandle by value.
// Asm: if the panel reports the handler present, remove it; the row is passed through unchanged.
// CONFIDENCE: MEDIUM
UCommonActivatablePanel* Panel = reinterpret_cast<UCommonActivatablePanel*>(NativeRecoveredObject_0x278);
if (Panel->HasInputActionHandler(InputActionRow))
{
	Panel->RemoveInputActionHandler(InputActionRow);
}
