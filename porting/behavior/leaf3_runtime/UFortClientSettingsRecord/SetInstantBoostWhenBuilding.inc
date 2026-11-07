// Behaviour snippet: UFortClientSettingsRecord::SetInstantBoostWhenBuilding (body only; spliced into the generated member function).
// Original body address: 0x141e3bb60 (thunk 0x142880d10)
// Offset evidence: MOV byte [this+0x128],DL; MOV byte [this+0x13c],1. +0x128 = LocalGamepadUserOptions (+0xf0) + 0x38 = Advanced (+0x10) + 0x28 = bInstantBoostWhenBuilding.
// Dirty flag +0x13c = bNativeGamepadOptionsDirty (declared in header).
// CONFIDENCE: HIGH
LocalGamepadUserOptions.Advanced.bInstantBoostWhenBuilding = NewValue;
bNativeGamepadOptionsDirty = true;
