// Behaviour snippet: UFortClientSettingsRecord::SetUseAdvancedOptions (body only; spliced into the generated member function).
// Original body address: 0x141e3cca0 (thunk 0x142883fa0)
// Offset evidence: MOV byte [this+0xfc],DL; MOV byte [this+0x13c],1. +0xfc = LocalGamepadUserOptions (+0xf0) + 0xc = Basic.bUseAdvancedOptions.
// Dirty flag +0x13c = bNativeGamepadOptionsDirty (declared in header).
// CONFIDENCE: HIGH
LocalGamepadUserOptions.Basic.bUseAdvancedOptions = NewUseAdvancedOptions;
bNativeGamepadOptionsDirty = true;
