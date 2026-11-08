// Original body 0x144db8b40 (thunk 0x14530e3a0). Offset evidence: no object field access. The receiver is NativeGetFortAssetManager() (0x141df62a0), and the body tail-calls NativeFindAssetByPrimaryId (0x141df6530) with TemplateId by reference.
// CONFIDENCE: HIGH
return UFortAssetManager::NativeGetFortAssetManager().NativeFindAssetByPrimaryId(TemplateId);
