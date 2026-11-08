// Original body: 0x144b2fe50 (thunk 0x1451b5060). Offset evidence: MOV RCX,[RCX+0x258] loads EquipButtonGroup (reflected Object at +0x258); XOR EDX,EDX sets Index 0; JMP 0x1407c1f20 tail-calls UFortCreativeServersView::NativeGetServerEntry1407c1f20(UObject* Context, int32 Index) and returns its RAX unchanged.
// The returned UObject* is reinterpreted to the declared UWidget* return type.
// CONFIDENCE: HIGH
static_assert(sizeof(UCommonButtonGroup) > 0, "EquipButtonGroup pointee is named for the UObject* conversion");
return reinterpret_cast<UWidget*>(UFortCreativeServersView::NativeGetServerEntry1407c1f20(EquipButtonGroup, 0));
