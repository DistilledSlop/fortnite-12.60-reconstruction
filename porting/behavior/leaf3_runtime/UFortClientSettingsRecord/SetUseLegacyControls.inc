// Behaviour snippet: UFortClientSettingsRecord::SetUseLegacyControls (body only; spliced into the generated member function).
// Original body address: 0x141e3ccb0 (thunk 0x142884270)
// Offset evidence: MOV byte [this+0x138],DL; MOV byte [this+0x13c],1. +0x138 = LocalGamepadUserOptions (+0xf0) + 0x10 + 0x38 = Advanced.bUseLegacyControls.
// Dirty flag +0x13c = bNativeGamepadOptionsDirty (declared in header).
// CONFIDENCE: HIGH
LocalGamepadUserOptions.Advanced.bUseLegacyControls = NewUseLegacyControls;
bNativeGamepadOptionsDirty = true;
