// Exec body fragment for 0x1451f76d0. Structure verified against disassembly; helper semantics are UNKNOWN (declarations.inc).
// Each read is guarded by the FFrame code pointer: when Stack.Code == nullptr the direct path is used, otherwise the
// property-driven path. Both paths are represented by the same helper call here, so their difference is not modelled.

static void ExecShowSimpleConfirmationDialog_CustomInput_1451f76d0(UObject* Context, FFrame& Stack, void* const ResultOut)
{
    // Read order mirrors the reflected parameter list in reverse index order (10 -> 1).
    UObject* InWorldContextObject = nullptr;  NativeReadParam_142ed9cf0(Stack, &InWorldContextObject);
    FUnknownIcon_142fdb590 Icon{};            NativeReadParam_142ed9cf0(Stack, &Icon);
    FText Title{};                            NativeDefaultTextOrName_142c0c240(&Title);  NativeReadParam_142ed9cf0(Stack, &Title);
    FText Message{};                          NativeDefaultTextOrName_142c0c240(&Message); NativeReadParam_142ed9cf0(Stack, &Message);
    FName ConfirmAction = NAME_None;          NativeReadParam_142ed9cf0(Stack, &ConfirmAction);
    FName DeclineAction = NAME_None;          NativeReadParam_142ed9cf0(Stack, &DeclineAction);
    UObject* AdditionalContent = nullptr;     NativeReadParam_142ed9cf0(Stack, &AdditionalContent);
    UObject* LeftAdditionalContent = nullptr; NativeReadParam_142ed9cf0(Stack, &LeftAdditionalContent);
    int32 CancelableRaw = 0;                  NativeReadParam_142ed9cf0(Stack, &CancelableRaw); // verified: 4-byte slot, tested != 0
    const bool Cancelable = CancelableRaw != 0;
    UObject* TargetPlayer = nullptr;          NativeReadParam_142ed9cf0(Stack, &TargetPlayer);

    UFortAsyncAction_ShowConfirmation_NUI* Result = NativeShowSimpleConfirmationDialogCustomInput_144b1aeb0(
        InWorldContextObject, &Icon, &Title, &Message, ConfirmAction, DeclineAction,
        AdditionalContent, LeftAdditionalContent, Cancelable, TargetPlayer);

    // Verified: the result is stored through the ResultOut pointer (r8 saved at entry, `mov %rax,(%r14)`).
    *static_cast<UFortAsyncAction_ShowConfirmation_NUI**>(const_cast<void*>(ResultOut)) = Result;
}
