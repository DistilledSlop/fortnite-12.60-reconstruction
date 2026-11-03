// Legacy CommonUI input dispatch: declarations. Names are descriptive (original source
// names not recovered). Address-suffixed names are project-local. Signatures follow the
// Ghidra parameter list; types marked "Native" are placeholders (layouts UNKNOWN, NOTES.md).

// Funnel 0x1407c8d40. Member of UCommonGlobalInputHandler (Ghidra param_1 = handler base).
// Returns the delegate out-bool on the success path; false otherwise.
bool UCommonGlobalInputHandler::NativeFunnel1407c8d40(FCommonInputEventNative* Event, int32 Priority,
    uint64 Param4, FCommonDelegateNative* Delegate);

// Global interface slots. InterfaceThis is the interface sub-object (handler base + 0x28).
void NativeGlobalSlot0x10_1407c8be0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint64 Arg5);
void NativeGlobalSlot0x18_1407cab10(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint64 Arg4);
void NativeGlobalSlot0x20_1407cc2b0(void* InterfaceThis, int32 Arg2, int32 Arg3, uint64 Arg4, uint64 Arg5);

// Called by slot 0x20 instead of the funnel (asm 1407cc35c: call 1407cc3b0). Body UNKNOWN.
void NativeHelper1407cc3b0(UCommonGlobalInputHandler* Self, int32 Arg2, int32 Arg3, uint64 Arg4);

// Interface thunks.
void* NativeGlobalSlot0x0_1407a5da8(void* InterfaceThis);      // asm: sub rcx,0x28; jmp 0x7a7d10 (dtor, UNKNOWN)
UCommonGlobalInputHandler* NativeGlobalOwner0x8_140695430(void* InterfaceThis); // asm: lea rax,[rcx-0x28]; ret
void* NativePanelSlot0x0_1407a5d84(void* InterfaceThis);       // asm: sub rcx,0x2d8; jmp 0x7a78d0 (UNKNOWN)
UCommonActivatablePanel* NativePanelOwner0x8_1408056ac(void* InterfaceThis); // asm: sub rcx,0x1d0; jmp 0x6a7c40

// External (owned by another worker). Not translated here.
bool UCommonGlobalInputHandler::NativeEligibility1407d0160(const void* RowData);
