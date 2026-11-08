// Behaviour snippet: UFortHUDCenterPopupMessageWidget::GetCenterPopupMessageState (body only; spliced into the generated member function).
// Original address: 0x144d79260 (thunk 0x145255aa0)
// Evidence: leaf; MOVZX EAX,byte [this+0x2c0] then RET. +0x2c0 lies in the class's own region (class min 0x298) and no reflected member covers it, so it is the typed member NativeRecoveredByte_0x2c0 (uint8). Returned as ECenterPopupMessageStateEnum (: uint8).
// CONFIDENCE: MEDIUM
return static_cast<ECenterPopupMessageStateEnum>(NativeRecoveredByte_0x2c0);
