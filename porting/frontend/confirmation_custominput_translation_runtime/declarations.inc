// Reflected: UFortAsyncAction_ShowConfirmation_NUI::ShowSimpleConfirmationDialog_CustomInput
// exec 0x1451f76d0, function flags 0x4022401. Return type is UFortAsyncAction_ShowConfirmation_NUI* (ReturnValue, offset 240).
// Parameter order below is the verified bytecode read order (Stack reads in disassembly of 0x1451f76d0).
// Object-kind parameters are declared as UObject* because their concrete classes are UNKNOWN (see NOTES.md).

// Exec entry (native thunk shape: Context, FFrame&, RESULT_DECL*). Verified: rdx=FFrame, r8=result pointer.
static void ExecShowSimpleConfirmationDialog_CustomInput_1451f76d0(UObject* Context, FFrame& Stack, void* const ResultOut);

// Native body: callee 0x144b1aeb0 (non-reflected, project-local name).
UFortAsyncAction_ShowConfirmation_NUI* NativeShowSimpleConfirmationDialogCustomInput_144b1aeb0(
    UObject* InWorldContextObject, const FUnknownIcon_142fdb590* Icon, const FText* Title, const FText* Message,
    FName ConfirmAction, FName DeclineAction, UObject* AdditionalContent, UObject* LeftAdditionalContent,
    bool Cancelable, UObject* TargetPlayer);

// Helpers used by the exec. Bodies are UNKNOWN; do not implement from this fragment.
struct FUnknownIcon_142fdb590; // struct at 0x142fdb590; name not resolved
void NativeReadParam_142ed9cf0(FFrame& Stack, void* OutValue);          // dispatches via table DAT_148262b60 (UNKNOWN)
void NativeReadParamProperty_142ed9d20(FFrame& Stack, void* Property, void* OutValue); // UNKNOWN
void NativeDefaultTextOrName_142c0c240(void* Out);                       // copies a value from 1407c2c80 (UNKNOWN)
