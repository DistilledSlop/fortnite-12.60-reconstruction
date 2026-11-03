// Legacy CommonUI input dispatch: declarations. Names are descriptive (original source
// names not recovered). Address-suffixed names are project-local. Signatures follow the
// Ghidra parameter list, corrected where asm shows a different argument list (NOTES.md
// "Divergences"). Types marked "Native" are placeholders (layouts UNKNOWN, NOTES.md).

// Funnel 0x1407c8d40. Member of UCommonGlobalInputHandler (Self = handler base).
// Returns the delegate out-bool on the success path; false otherwise.
// asm: rcx=Self, rdx=Event, r8d=Priority, r9=Param4, [rsp+0x28]=Delegate.
bool UCommonGlobalInputHandler::NativeFunnel1407c8d40(FCommonInputEventNative* Event, int32 Priority,
    uint64 Param4, FCommonDelegateNative* Delegate);

// Global interface slots. InterfaceThis is the interface sub-object (handler base + 0x28).
void NativeGlobalSlot0x10_1407c8be0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint64 Arg5);
void NativeGlobalSlot0x18_1407cab10(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint64 Arg4);
void NativeGlobalSlot0x20_1407cc2b0(void* InterfaceThis, int32 Arg2, int32 Arg3, uint64 Arg4, uint64 Arg5);
void NativeGlobalSlot0x30_140800b70(void* InterfaceThis, int32 Arg2, const void* Row);

// Called by global slot 0x20 instead of the funnel (asm 7cc35c). Table-driven variant of the funnel.
// asm args: rcx=Self, edx=Arg2 (table index), r8d=Arg3, r9=Arg4, [rsp+0x20]=Arg5, [rsp+0x28]=Delegate.
// Decompile param_4 is not referenced in the body.
bool UCommonGlobalInputHandler::NativeHelper1407cc3b0(uint32 TableIndex, int32 Arg3, uint64 Arg4, uint64 Arg5,
    FCommonDelegateNative* Delegate);

// Panel interface slots. InterfaceThis is the panel interface sub-object (panel + 0x2d8 offset per
// NativePanelOwner0x8). Signatures from the Ghidra parameter list; asm argument check NOT done.
bool NativePanelSlot0x10_1407c87e0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint8* Arg5);
bool NativePanelSlot0x18_1407ca7f0(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint8* Arg4);
bool NativePanelSlot0x20_1407cbf10(void* InterfaceThis, uint32 TableIndex, int32 Arg3, uint8* Arg5);
void NativePanelSlot0x30_140800860(void* InterfaceThis, int32 Arg2, const void* Row);

// Interface thunks.
void* NativeGlobalSlot0x0_1407a5da8(void* InterfaceThis);      // asm: sub rcx,0x28; jmp 0x7a7d10 (dtor, UNKNOWN)
UCommonGlobalInputHandler* NativeGlobalOwner0x8_140695430(void* InterfaceThis); // asm: lea rax,[rcx-0x28]; ret
void* NativePanelSlot0x0_1407a5d84(void* InterfaceThis);       // asm: sub rcx,0x2d8; jmp 0x7a78d0 (UNKNOWN)
UCommonActivatablePanel* NativePanelOwner0x8_1408056ac(void* InterfaceThis); // asm: sub rcx,0x1d0; jmp 0x6a7c40

// External (owned by another worker). Not translated here. Signature corrected: asm 7c8ecc-7c8ed2
// passes (this, record+0x10), not the row pointer.
bool UCommonGlobalInputHandler::NativeEligibility1407d0160(const void* RecordKey10);
