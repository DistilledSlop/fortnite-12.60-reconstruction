// Behaviour snippet: UFortHUDCenterPopupMessageWidget::GetCenterPopupMessageState (body only; spliced into the generated member function).
// Original address: 0x144d79260 (thunk 0x145255aa0).
// asm: MOVZX EAX,byte [+0x2c0]; RET (enum byte). No reflected member covers +0x2c0.
// Return type is the enum named only as Enum in the packet; body converts via decltype of this function (no enum name recovered).
// CONFIDENCE: MEDIUM
return static_cast<decltype(this->GetCenterPopupMessageState(0))>(NativeRecoveredByte_0x2c0);
