// Legacy CommonUI input dispatch: definitions. Branch structure follows the Ghidra decompile
// (porting/generated/decomp-imports/Imports/Funnel__1407c8d40.c and slot files). Helpers named
// NativeHelper<addr> are UNKNOWN and not translated. Addresses and the UNKNOWN list: NOTES.md.

// ---- Interface thunks (asm-verified) ----

UCommonGlobalInputHandler* NativeGlobalOwner0x8_140695430(void* InterfaceThis)
{
    return reinterpret_cast<UCommonGlobalInputHandler*>(static_cast<uint8*>(InterfaceThis) - 0x28);
}

UCommonActivatablePanel* NativePanelOwner0x8_1408056ac(void* InterfaceThis)
{
    // asm: sub rcx,0x1d0; jmp 0x6a7c40. 0x6a7c40 returns rcx-0x108 (prior NOTES.md pass): net -0x2d8.
    return reinterpret_cast<UCommonActivatablePanel*>(static_cast<uint8*>(InterfaceThis) - 0x2d8);
}

// ---- Funnel 0x1407c8d40 (structure from decompile; helper semantics UNKNOWN) ----

bool UCommonGlobalInputHandler::NativeFunnel1407c8d40(FCommonInputEventNative* Event, int32 Priority,
    uint64 Param4, FCommonDelegateNative* Delegate)
{
    bool bResult = false;
    NativeHelper142ee3b30(this);      // UNKNOWN temp init (decompile FUN_142ee3b30)
    int32 Index = 0;
    while (Index < NativeField58())   // decompile: [this+0x58] count, records at [this+0x50]
    {
        FCommonLegacyGlobalAction* Record = NativeRecordAt1407c8d40(Index); // base+0x50 + Index*0x90
        const void* RowData = NativeHelper14078c640(Record);   // UNKNOWN; GetInputActionData row lookup
        bool bCandidate = RowData != nullptr && !(Record->Field18 < NativeField68());
        if (bCandidate)
            bCandidate = NativeHelper1407cf950(RowData, Event, NativeHelper142ee5dd0(NativeField60Ref())) != 0;
        bool bTarget = false;
        if (bCandidate)
        {
            // decompile: FUN_142ee7880(record+0x10) validity; FUN_143d5a140 resolves the object (UNKNOWN).
            if (NativeHelper142ee7880(&Record->Field10))
                bTarget = NativeHelper143d5a140(&Record->Field10) != nullptr;
        }
        if (!bTarget)
        {
            // decompile FUN_1407f0790(this+0x50, Index, 1, 1); then Index is not advanced.
            NativeHelper1407f0790(NativeRecordArrayRef(), Index, 1, 1);
            continue;
        }
        if (NativeEligibility1407d0160(RowData))
        {
            // decompile CONCAT44: low 32 bits of the 64-bit key become Priority, high bits unchanged.
            uint64 Key = (Record->Key & 0xffffffff00000000ull) | static_cast<uint32>(Priority);
            uint8 bOut = 0;
            // Delegate: target = Delegate->Obj ? Delegate->Obj : &Delegate->Inline (UNKNOWN layout).
            // Invocation through *Delegate (function pointer, UNKNOWN calling convention).
            bool bInvoked = NativeDelegateInvoke1407c8d40(Delegate, Record, RowData, &Key, Param4, &bOut);
            if (bInvoked)
            {
                bResult = bOut != 0;   // decompile abStack_138[0]
                NativeReleaseArraysAndHeld1407c8d40(); // decompile lines: plStack_98, plStack_b0, plStack_c0, plStack_d8 cleanup
                NativeHelper14079a3a0(Event);          // decompile FUN_14079a3a0(param_2); UNKNOWN
                if (Delegate->Fn != nullptr)            // decompile *param_5 != 0 (UNKNOWN field)
                    NativeDelegateFinish1407c8d40(Delegate);  // vtable +0x10 (UNKNOWN)
                return bResult;
            }
        }
        ++Index;   // not eligible or declined: next record
    }
    NativeReleaseArraysAndHeld1407c8d40();
    // decompile tail: Event word 2 intrusive release (refcount dec; zero -> vtable[0] destroy,
    // weak dec -> vtable[1](obj,1)). Object type UNKNOWN.
    NativeReleaseIntrusive(Event->Ref);
    return bResult;
}

// ---- Global slots (decompile + asm call targets) ----

// 0x10 1407c8be0: asm calls 1407c8d40 at 7c8cbf. Arg2 feeds the UNKNOWN guard (NativeHelper1407ae9d0).
void NativeGlobalSlot0x10_1407c8be0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint64 Arg5)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    NativeHelper142ee3b30(Self);        // UNKNOWN (decompile FUN_142ee3b30 on this-0x28)
    NativeHelper1407ae9d0();            // UNKNOWN guard, args UNKNOWN (asm: &puStack_58, auStack_a8; vtable 1459feea8)
    FCommonInputEventNative Event = NativeCopyEvent(Arg3); // copy words 0,1 and addref word 2
    Self->NativeFunnel1407c8d40(&Event, Arg4, Arg5);
    NativeGuardRelease();               // UNKNOWN (vtable+0x10 when set)
    NativeReleaseIntrusive(Arg3->Ref);  // release of Arg3[2] (decompile tail on plVar3)
}

// 0x18 1407cab10: asm calls 1407c8d40 at 7cabd3. Same shape; guard helper 1407ae9a0 (vtable 1459feee8).
void NativeGlobalSlot0x18_1407cab10(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint64 Arg4)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    NativeHelper142ee3b30(Self);
    NativeHelper1407ae9a0();            // UNKNOWN guard (vtable 1459feee8)
    FCommonInputEventNative Event = NativeCopyEvent(Arg2);
    Self->NativeFunnel1407c8d40(&Event, Arg3, Arg4);
    NativeGuardRelease();
    NativeReleaseIntrusive(Arg2->Ref);
}

// 0x20 1407cc2b0: NOT the funnel. asm 7cc35c calls 1407cc3b0. Arg5 stored to an UNKNOWN local.
void NativeGlobalSlot0x20_1407cc2b0(void* InterfaceThis, int32 Arg2, int32 Arg3, uint64 Arg4, uint64 Arg5)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    NativeHelper142ee3b30(Self);
    NativeHelper1407ae9a0();            // UNKNOWN guard
    NativeHelper1407cc3b0(Self, Arg2, Arg3, Arg4);
    NativeGuardRelease();
    (void)Arg5;                         // decompile stores Arg5 into a local; use UNKNOWN
}

// ---- Not translated (UNKNOWN) ----
// Global 0x0 1407a5da8 (dtor thunk), global 0x30 140800b70 (state lookup),
// panel 0x0 1407a5d84 (dtor thunk), panel 0x10 1407c87e0, 0x18 1407ca7f0, 0x20 1407cbf10, 0x30 140800860.
// Tick bodies 140802440 and 140802200 are out of scope.
