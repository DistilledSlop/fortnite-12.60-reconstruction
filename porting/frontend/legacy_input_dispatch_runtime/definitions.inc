// Legacy CommonUI input dispatch: definitions.
// Tags: [asm] = branch and call target checked against disassembly (image-125 windows, see NOTES.md);
//       [dec] = structure from the Ghidra decompile only, not asm-checked.
// Helpers named NativeHelper<addr> and NativeXxx<addr> are UNKNOWN (not translated).
// Addresses and the UNKNOWN list: NOTES.md.

// ---- Interface thunks ([asm]) ----

UCommonGlobalInputHandler* NativeGlobalOwner0x8_140695430(void* InterfaceThis)
{
    return reinterpret_cast<UCommonGlobalInputHandler*>(static_cast<uint8*>(InterfaceThis) - 0x28);
}

UCommonActivatablePanel* NativePanelOwner0x8_1408056ac(void* InterfaceThis)
{
    // asm: sub rcx,0x1d0; jmp 0x6a7c40. 0x6a7c40 returns rcx-0x108 (prior NOTES.md pass): net -0x2d8.
    return reinterpret_cast<UCommonActivatablePanel*>(static_cast<uint8*>(InterfaceThis) - 0x2d8);
}

// ---- Funnel 0x1407c8d40 ([asm] branch structure, checked against objdump 7c8d40..7c930e) ----
// Loop over records at this+0x50, count this+0x58, record stride 0x90.
// Skip (advance, no removal): record+0x18 < this+0x68 (unsigned), or row null, or filter false.
// Remove (index not advanced): target invalid (valid test or first qword of resolved object == 0).
// Advance: eligibility false, or delegate returns false.
// Delegate: word0 = fn, word2 = object (if null, object is inline at +0x20).

bool UCommonGlobalInputHandler::NativeFunnel1407c8d40(FCommonInputEventNative* Event, int32 Priority,
    uint64 Param4, FCommonDelegateNative* Delegate)
{
    // [asm 7c8d9c] FUN_142ee3b30(temp, 0) on a function-local temp. Not this (see NOTES.md divergences).
    FNativeTemp FunnelTemp{};
    NativeHelper142ee3b30(&FunnelTemp, 0);
    bool bResult = false;
    int32 Index = 0;
    while (Index < NativeField58())                                            // [asm 7c8df1] cmp [this+0x58]
    {
        FCommonLegacyGlobalAction* Record = NativeRecordAt1407c8d40(Index);    // [asm 7c8dfb] [this+0x50]+Index*0x90
        const void* RowData = NativeHelper14078c640(Record, kRowMissingMsg);  // [asm 7c8e15]
        // [asm 7c8e1a-7c8e32] skip when record+0x18 < this+0x68 (unsigned) or row null. Advance only.
        if (Record->Field18 < NativeField68() || RowData == nullptr)
        {
            ++Index;
            continue;
        }
        // [asm 7c8e3c-7c8e51] filter(row, Event, FUN_142ee5dd0(this+0x60)) false: advance only (no removal).
        if (!NativeHelper1407cf950(RowData, Event, NativeHelper142ee5dd0(NativeField60Ref())))
        {
            ++Index;
            continue;
        }
        // [asm 7c8e57-7c8e88] target test on record+0x10.
        bool bValid = false;
        bool bOutHeld = false;
        FNativeOutRef Out{};                                                    // [asm rbp+0x8]
        if (NativeHelper142ee7880(&Record->Field10))                            // [asm 7c8e5e]
        {
            bOutHeld = true;                                                    // [asm 7c8e6a] or r15d,1
            void* Res = NativeHelper143d5a140(NativeHelper142ee5dd0(&Record->Field10), &Out);  // [asm 7c8e6e-7c8e7a]
            bValid = *static_cast<const uint64*>(Res) != 0;                    // [asm 7c8e7f] cmpq $0,(%rax)
        }
        if (bOutHeld)
            NativeReleaseSharedRef(Out.Object);                                 // [asm 7c8e91-7c8ec2] release [rbp+0x10]
        if (!bValid)
        {
            // [asm 7c8f3d-7c8f4c] FUN_1407f0790(this+0x50, Index, 1, 1); index NOT advanced [asm 7c8f58-7c8f64].
            NativeHelper1407f0790(NativeRecordArrayRef(), Index, 1, 1);
            continue;
        }
        // [asm 7c8ecc-7c8ede] eligibility(this, record+0x10) false: advance.
        if (!NativeEligibility1407d0160(&Record->Field10))
        {
            ++Index;
            continue;
        }
        // [asm 7c8ee0-7c8f09] Key = (record address & 0xffffffff00000000) | Priority. The high dword comes
        // from the RECORD POINTER (asm 7c8e10 mov %rbx,0x38(rsp)), not a record field.
        uint64 Key = (reinterpret_cast<uintptr>(Record) & 0xffffffff00000000ull) | static_cast<uint32>(Priority);
        const void* RowSlot = RowData;                                          // [asm 7c8e21] address passed as arg 3
        uint8 bOut = 0;                                                         // [asm 7c8efd] movb $0
        // [asm 7c8ef6-7c8f09] target = word2 ? word2 : &Delegate+0x20; this-adjust = vtable[+8](target).
        void* Target = Delegate->Word2 != nullptr ? Delegate->Word2 : &Delegate->Inline;
        void* ThisPtr = NativeVcallSlot1(Target);                               // UNKNOWN vtable slot 1, U8
        // [asm 7c8f0c-7c8f2f] Delegate->Word0(ThisPtr, Record, &RowSlot, &Key, Param4, &bOut)
        bool bInvoked = NativeDelegateInvoke1407c8d40(Delegate, ThisPtr, Record, &RowSlot, &Key, Param4, &bOut);
        if (!bInvoked)                                                          // [asm 7c8f34] jne
        {
            ++Index;                                                            // [asm 7c8f36-7c8f64] advance
            continue;
        }
        // ---- Success path [asm 7c8fa2-7c9145] ----
        bResult = bOut != 0;                                                    // [asm 7c8fae] movzbl 0x30(rsp)
        NativeReleaseTemps1407c8d40();                                          // [dec] cleanup arrays (asm 7c8fa5-7c9118)
        NativeHelper14079a3a0(Event);                                           // [asm 7c9122] rcx = Event
        // [asm 7c912a-7c913e] if Delegate->Word0 != 0: target->vtable[+0x10]().
        if (Delegate->Word0 != nullptr)
            NativeDelegateFinish1407c8d40(Target);                              // target = word2 or inline
        return bResult;                                                         // [asm 7c9141-7c9145] Event word2 NOT released here
    }
    // ---- Fall-through [asm 7c914a-7c92fb] ----
    NativeReleaseTemps1407c8d40();                                              // [dec]
    NativeReleaseSharedRef(Event->Ref);                                         // [asm 7c92a4-7c92c8] Event word2
    if (Delegate->Word0 != nullptr)                                             // [asm 7c92de] cmpq $0,(%r14)
        NativeDelegateFinish1407c8d40(Delegate->Word2 != nullptr ? Delegate->Word2 : &Delegate->Inline); // [asm 7c92e4-7c92f6]
    return false;                                                               // [asm 7c92f9] xor al
}

// ---- Global slot 0x10 1407c8be0 ----
// [asm 7c8be0-7c8cc4]: Self = this-0x28 (rdi); temp via FUN_142ee3b30(temp, Self) (rdx=Self, asm 7c8c1c).
// Guard FUN_1407ae9d0 (vtable 1459feea8). Event = copy of Arg3 words 0..2 with addref word2 (asm 7c8c85-7c8ca4).
// Delegate local {word0 = LAB_1407ad820, word2 = 0} (asm 7c8c47-7c8c56). Funnel call args: rdx=&Event,
// r8d=Arg4 (Priority), r9=Arg5 (Param4), [rsp+0x20]=&Delegate (asm 7c8ca7-7c8cbf).
void NativeGlobalSlot0x10_1407c8be0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint64 Arg5)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    FNativeTemp Temp{};
    NativeHelper142ee3b30(&Temp, Self);                                         // [asm 7c8c1c] rdx = Self
    FNativeGuard Guard{};
    NativeHelper1407ae9d0(&Guard, &Temp, Arg2);                                 // [dec] guard, asm 7c8c80
    FCommonInputEventNative Event = NativeCopyEvent(Arg3);                      // [asm 7c8c85-7c8ca4] addref word2
    FCommonDelegateNative Delegate{};
    Delegate.Word0 = reinterpret_cast<void*>(0x1407ad820);                      // [asm 7c8c47] LAB_1407ad820
    Delegate.Word2 = nullptr;                                                   // [asm 7c8c56]
    Self->NativeFunnel1407c8d40(&Event, Arg4, Arg5, &Delegate);
    if (Guard.Word90 != nullptr)                                                // [asm 7c8cc4] cmpq $0,0x90(rsp)
        NativeGuardFinish(&Guard);                                              // [dec] vtable+0x10
    NativeReleaseSharedRef(Arg3->Ref);                                          // [dec] release Arg3 word2
}

// ---- Global slot 0x18 1407cab10 [dec] ----
// Asm check of the funnel call arguments (Delegate on stack) NOT done for this slot.
void NativeGlobalSlot0x18_1407cab10(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint64 Arg4)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    FNativeTemp Temp{};
    NativeHelper142ee3b30(&Temp, Self);                                         // [dec] FUN_142ee3b30(&temp, Self)
    FNativeGuard Guard{};
    NativeHelper1407ae9a0(&Guard, &Temp);                                       // [dec] guard, vtable 1459feee8
    FCommonInputEventNative Event = NativeCopyEvent(Arg2);
    FCommonDelegateNative* Delegate = NativeDelegate1407cab10();               // UNKNOWN: 5th arg (stack) not asm-checked
    Self->NativeFunnel1407c8d40(&Event, Arg3, Arg4, Delegate);
    if (Guard.Word90 != nullptr)
        NativeGuardFinish(&Guard);
    NativeReleaseSharedRef(Arg2->Ref);
}

// ---- Global slot 0x20 1407cc2b0 ([asm] call structure) ----
// Does NOT call the funnel. asm 7cc2ec: FUN_142ee3b30(temp, Self) with rdx=Self. Guard FUN_1407ae9a0 (asm 7cc33d).
// asm 7cc35c: call 1407cc3b0 with rcx=Self, edx=Arg2, r8d=Arg3, r9=Arg4, [rsp+0x20]=Arg5 (entry [rsp+0x28]),
// [rsp+0x28]=&Delegate local {word0 = 0x7ad780, word2 = 0 (asm 7cc338, 7cc356)}.
void NativeGlobalSlot0x20_1407cc2b0(void* InterfaceThis, int32 Arg2, int32 Arg3, uint64 Arg4, uint64 Arg5)
{
    UCommonGlobalInputHandler* Self = NativeGlobalOwner0x8_140695430(InterfaceThis);
    FNativeTemp Temp{};
    NativeHelper142ee3b30(&Temp, Self);                                         // [asm 7cc2ec] rdx = Self
    FNativeGuard Guard{};
    NativeHelper1407ae9a0(&Guard, &Temp);                                       // [asm 7cc33d]
    FCommonDelegateNative Delegate{};
    Delegate.Word0 = reinterpret_cast<void*>(0x1407ad780);                      // [asm 7cc31c, 7cc338]
    Delegate.Word2 = nullptr;                                                   // [asm 7cc323]
    Self->NativeHelper1407cc3b0(Arg2, Arg3, Arg4, Arg5, &Delegate);            // [asm 7cc35c]
    if (Guard.Word90 != nullptr)                                                // [asm 7cc361] cmpq $0,0x80(rsp)
        NativeGuardFinish(&Guard);
}

// ---- Helper 1407cc3b0 (called by slot 0x20) [dec] ----
// Table-driven funnel: table index Arg2 < 0xb selects key K = DAT_1482651b0[Arg2] (8 bytes) and object
// T = table[Arg2] (addref). Active = FUN_142ee5dd0(this+0x60) (not FUN_14081b480 as in the funnel).
// Records at this+0x50, count this+0x58, stride 0x90. Enter when record+0x18 >= this+0x68.
// Row pair pointer selected by mode (row+0x38 / row+0xe0+idx*0xa8 / row+0x380), key compared to K (8 bytes).
// Eligibility FUN_1407d0160(this, record+0x10). Invocation args: (ThisPtr, Record, &RowData, &Key, Arg5, &bOut).
// Key high dword never written in the decompile (auStack_128[1]); UNKNOWN.
bool UCommonGlobalInputHandler::NativeHelper1407cc3b0(uint32 TableIndex, int32 Arg3, uint64 Arg4, uint64 Arg5,
    FCommonDelegateNative* Delegate)
{
    FNativeTemp Temp{};
    NativeHelper142ee3b30(&Temp, 0);                                            // [dec] FUN_142ee3b30(&temp, 0)
    uint8 bOut = 0;
    FNativeTableObj TableObj{};
    if (TableIndex < 11)
    {
        FNativeKey TableKey = NativeTableKey(TableIndex);                       // DAT_1482651b0 + idx*0x18
        TableObj = NativeTableObject(TableIndex);                               // +0x1482651c0, addref
        int32 Index = 0;
        while (Index < NativeField58())
        {
            FCommonLegacyGlobalAction* Record = NativeRecordAt1407c8d40(Index);
            const void* RowData = NativeHelper14078c640(Record, kRowMissingMsg);
            bool bEnter = Record->Field18 >= NativeField68();                   // [dec] FUN_1407cc3b0 uses <=
            if (bEnter && RowData != nullptr)
            {
                // [dec] Active = FUN_142ee5dd0(this+0x60); null selects the default row+0x38 pair.
                void* Active = NativeHelper142ee5dd0(NativeField60Ref());
                FNativeKey RowKey = NativePairKeyForMode(RowData, Active);     // [dec] pair word0 (8 bytes)
                if (RowKey == TableKey)                                         // both dwords equal
                {
                    bool bValid = false;
                    bool bOutHeld = false;
                    FNativeOutRef Out{};
                    if (NativeHelper142ee7880(&Record->Field10))
                    {
                        bOutHeld = true;
                        void* Res = NativeHelper143d5a140(NativeHelper142ee5dd0(&Record->Field10), &Out);
                        bValid = *static_cast<const uint64*>(Res) != 0;
                    }
                    if (bOutHeld)
                        NativeReleaseSharedRef(Out.Object);
                    if (bValid)
                    {
                        if (NativeEligibility1407d0160(&Record->Field10))
                        {
                            FNativeKeyPartial Key{};
                            Key.Low = Arg3;                                     // [dec] auStack_128[0] = Arg3
                            Key.High = NativeUnknownHigh();                     // UNKNOWN: never written in decompile
                            const void* RowSlot = RowData;
                            void* ThisPtr = NativeVcallSlot1(Delegate->Word2 != nullptr ? Delegate->Word2 : &Delegate->Inline);
                            bool bInvoked = NativeDelegateInvoke1407cc3b0(Delegate, ThisPtr, Record, &RowSlot, &Key, Arg5, &bOut);
                            if (bInvoked)
                            {
                                NativeReleaseSharedRef(TableObj.Object);
                                NativeReleaseTemps1407cc3b0();                  // [dec] cleanup, no FUN_14079a3a0 here
                                if (Delegate->Word0 != nullptr)
                                    NativeDelegateFinish1407c8d40(Delegate->Word2 != nullptr ? Delegate->Word2 : &Delegate->Inline);
                                return bOut != 0;
                            }
                        }
                        ++Index;
                        continue;
                    }
                    NativeHelper1407f0790(NativeRecordArrayRef(), Index, 1, 1); // [dec] removal, no advance
                    continue;
                }
            }
            ++Index;
        }
        NativeReleaseSharedRef(TableObj.Object);
    }
    NativeReleaseTemps1407cc3b0();
    if (Delegate->Word0 != nullptr)
        NativeDelegateFinish1407c8d40(Delegate->Word2 != nullptr ? Delegate->Word2 : &Delegate->Inline);
    return false;
}

// ---- Global slot 0x30 140800b70 [dec] ----
// Void. Enters only if this+0x38 (Self+0x60, U3 field) is valid. Pair = row pair selected by mode of
// FUN_142ee5dd0(this+0x38). Then owner = *(this-8) (Self+0x20): if the registration check holds,
// FUN_1407cac50(owner, Arg2, &pair-copy, 0). Temps released afterwards.
void NativeGlobalSlot0x30_140800b70(void* InterfaceThis, int32 Arg2, const void* Row)
{
    if (!NativeHelper142ee7880(static_cast<uint8*>(InterfaceThis) + 0x38))      // [dec] this+0x38
        return;
    void* ModeObj = NativeHelper142ee5dd0(static_cast<uint8*>(InterfaceThis) + 0x38);
    UCommonGlobalInputHandler* Owner = *reinterpret_cast<UCommonGlobalInputHandler* const*>(static_cast<uint8*>(InterfaceThis) - 8);
    if (Owner == nullptr)
        return;
    void* Reg = NativeHelper14080e600();                                        // UNKNOWN helper
    if (NativeRegistrationCheck(Owner, Reg))                                    // [dec] index <= count, slot == Reg+0x30
    {
        // [dec] Pair is copied and its object addref'd only inside this block (object word2 +1, +1 in decompile).
        FNativeDispatchPair Pair = NativeSelectPairByMode(Row, ModeObj);        // default row+0x38
        NativeHelper1407cac50(Owner, Arg2, &Pair, 0);
        NativeReleaseTemps1400800b70();
        NativeReleaseSharedRef(Pair.Object);
    }
}

// ---- Panel slot 0x10 1407c87e0 [dec] ----
// InterfaceThis-0x2d8 = panel owner. Flag byte at this-0x50 selects the panel path. Record stride 0x98,
// base this+0x88, count this+0xb0, bitset inline at this+0x98 or heap at this+0xa8. Iterates set bits.
// Arg4==0 path: writes record 16-byte key to this+0xe8..0xf8, sets resolved object fields.
// Arg4==1 path: FUN_1407cf0f0 gate, then compare (this+0x18 float) with row-state float (+0x1c), then
// calls the panel slot 0x30 through vtable slot 0x30 (this, Arg2, row, 0).
bool NativePanelSlot0x10_1407c87e0(void* InterfaceThis, int32 Arg2, const FCommonDispatchPairNative* Arg3,
    int32 Arg4, uint8* Arg5)
{
    UCommonActivatablePanel* Owner = NativePanelOwner0x8_1408056ac(InterfaceThis);
    if (NativeFlagAtMinus0x50(InterfaceThis) != 0)                              // [dec] param_1[-10]
    {
        NativeOwnerVirtual0x290(Owner);                                         // [dec] (*owner)+0x290
        void* Active = NativeHelper14081b480();                                 // UNKNOWN
        if (Active != nullptr)
        {
            // Start = lowest set bit index of the record bitset, clamped to Count (UNKNOWN helper, [dec]).
            int32 Count = NativePanelCount(InterfaceThis);                      // this+0xb0
            for (int32 It = NativeBitSetFirstIndex(InterfaceThis, Count); It < Count;
                 It = NativeBitSetNextIndex(InterfaceThis, It))                 // FUN_140687d00 advance, UNKNOWN
            {
                FCommonLegacyPanelAction* Record = NativePanelRecordAt(InterfaceThis, It);  // base + It*0x98
                const void* RowX = NativeHelper14078c640(Record, kRowMissingMsg);
                if (RowX == nullptr)
                    continue;
                uint8* FlagPtr = NativeRowFlagPtrByMode(RowX, Active);          // 0x51 / 0xf9+idx*0xa8 / 0x399
                if (!NativeHelper1407cf950(RowX, Arg3, Active) || *FlagPtr == 0)
                    continue;
                uint8 Cls = NativeHelper1407c5190(Record->Field60Ref(), Active);  // FUN_1407c5190(record+0x60, mode)
                if ((Cls & 0xfd) == 0)
                {
                    if (Arg4 == 0)
                    {
                        NativeHelper1407ae640(Owner);
                        if (NativeHelper142ee7880(static_cast<uint8*>(InterfaceThis) + 0xd8))
                        {
                            // [dec] copy record words 0..3 (16 bytes) to this+0xe8..0xf8.
                            NativeCopy16(static_cast<uint8*>(InterfaceThis) + 0xe8, Record);
                            void* Ctx = NativeHelper142ee5dd0(static_cast<uint8*>(InterfaceThis) + 0xd8);
                            NativeSetCtxOwner(Ctx, Owner, NativeHelper142ec9900(Owner, NativeHelper140806b60()));
                        }
                        NativeHelper14079a3a0(Arg3);
                        return true;
                    }
                    if (Arg4 == 1)
                    {
                        if (NativeHelper1407cf0f0(Owner, Record))
                        {
                            void* Active2 = NativeHelper14081b480(NativeOwnerVirtual0x290(Owner));
                            if (Active2 != nullptr)
                            {
                                void* Rec2 = NativeHelper1407c20f0(RowX, Active2);
                                if (NativeByteAt19(Rec2) != 0 && NativeFloat18(InterfaceThis) <= NativeFloat1c(Rec2)
                                    && NativeFloat1c(Rec2) != NativeFloat18(InterfaceThis))
                                {
                                    NativeHelper1407ae640(Owner);
                                    NativePanelSlot0x30_140800860(InterfaceThis, Arg2, RowX);  // [dec] vtable+0x30
                                    NativeHelper14079a3a0(Arg3);
                                    return true;
                                }
                            }
                            NativeHelper14079a3a0(Arg3);
                            return true;
                        }
                    }
                    NativeHelper14079a3a0(Arg3);
                    return false;
                }
                if (Cls == 1 && Arg4 == 0)
                {
                    if (NativeHelper1407bc6e0(Record->Field28Ref(), Arg5))
                    {
                        NativeHelper14079a3a0(Arg3);
                        return true;
                    }
                }
            }
        }
        if (NativeByte28(InterfaceThis) != 0)                                   // [dec] param_1[5]
        {
            *Arg5 = 0;
            NativeReleaseSharedRef(Arg3->Ref);
            return true;
        }
    }
    NativeReleaseSharedRef(Arg3->Ref);
    return false;
}

// ---- Panel slot 0x18 1407ca7f0 [dec] ----
// Same frame as 0x10 but Arg3 must be 0 and the row flag must be 0 (0x10 requires non-zero).
// No key compare. On Cls in {0,2}: resolves handle at record+0x74 (FUN_142ee5dd0), registration check
// (FUN_140811c70), FUN_1407f17d0(resolved), then FUN_1407bc6e0(record+0x28, Arg4), FUN_14079a3a0(Arg2).
bool NativePanelSlot0x18_1407ca7f0(void* InterfaceThis, const FCommonDispatchPairNative* Arg2, int32 Arg3,
    uint8* Arg4)
{
    if (NativeFlagAtMinus0x50(InterfaceThis) != 0)
    {
        NativeOwnerVirtual0x290(NativePanelOwner0x8_1408056ac(InterfaceThis));
        void* Active = NativeHelper14081b480();
        if (Active != nullptr)
        {
            int32 Count = NativePanelCount(InterfaceThis);
            for (int32 It = NativeBitSetFirstIndex(InterfaceThis, Count); It < Count;
                 It = NativeBitSetNextIndex(InterfaceThis, It))
            {
                FCommonLegacyPanelAction* Record = NativePanelRecordAt(InterfaceThis, It);
                const void* RowX = NativeHelper14078c640(Record, kRowMissingMsg);
                if (RowX == nullptr)
                    continue;
                uint8* FlagPtr = NativeRowFlagPtrByMode(RowX, Active);
                if (NativeHelper1407cf950(RowX, Arg2, Active) && *FlagPtr == 0 && Arg3 == 0)
                {
                    uint8 Cls = NativeHelper1407c5190(Record->Field60Ref(), Active);
                    if ((Cls & 0xfd) == 0)
                    {
                        uint64 Handle = Record->Field74;                        // [dec] *(record+0x74), 8 bytes
                        void* Obj = NativeHelper142ee5dd0(&Handle);
                        if (Obj != nullptr)
                        {
                            void* Reg = NativeHelper140811c70();
                            if (NativeRegistrationCheck2(Obj, Reg))
                                NativeHelper1407f17d0(Obj);                     // UNKNOWN
                        }
                        NativeHelper1407bc6e0(Record->Field28Ref(), Arg4);
                        NativeHelper14079a3a0(Arg2);
                        return true;
                    }
                }
            }
        }
        if (NativeByte28(InterfaceThis) != 0)
        {
            *Arg4 = 0;
            NativeReleaseSharedRef(Arg2->Ref);
            return true;
        }
    }
    NativeReleaseSharedRef(Arg2->Ref);
    return false;
}

// ---- Panel slot 0x20 1407cbf10 [dec] ----
// Table-driven (like helper 1407cc3b0). Requires flag at this-0x50 != 0 and TableIndex < 11. Key match
// (8 bytes) between table entry and the row pair selected by mode. Arg3 must be 0 and row flag must be 0.
// Success on Cls in {0,2} returns true after FUN_1407bc6e0(record+0x28, Arg5); Cls==1 same call.
// No filter call and no FUN_14079a3a0 in this slot. Branch nesting for the Cls==1 path inferred from the
// decompile layout (not asm-checked).
bool NativePanelSlot0x20_1407cbf10(void* InterfaceThis, uint32 TableIndex, int32 Arg3, uint8* Arg5)
{
    if (NativeFlagAtMinus0x50(InterfaceThis) == 0)
        return false;
    if (TableIndex < 11)
    {
        FNativeKey TableKey = NativeTableKey(TableIndex);
        FNativeTableObj TableObj = NativeTableObject(TableIndex);               // addref
        NativeOwnerVirtual0x290(NativePanelOwner0x8_1408056ac(InterfaceThis));
        void* Active = NativeHelper14081b480();
        if (Active != nullptr)
        {
            int32 Count = NativePanelCount(InterfaceThis);
            for (int32 It = NativeBitSetFirstIndex(InterfaceThis, Count); It < Count;
                 It = NativeBitSetNextIndex(InterfaceThis, It))
            {
                FCommonLegacyPanelAction* Record = NativePanelRecordAt(InterfaceThis, It);
                const void* RowX = NativeHelper14078c640(Record, kRowMissingMsg);
                if (RowX == nullptr)
                    continue;
                uint8* FlagPtr = NativeRowFlagPtrByMode(RowX, Active);
                FNativeKey RowKey = NativePairKeyForMode(RowX, Active);
                if (RowKey == TableKey && *FlagPtr == 0 && Arg3 == 0)
                {
                    uint8 Cls = NativeHelper1407c5190(Record->Field60Ref(), Active);
                    if ((Cls & 0xfd) == 0)
                    {
                        NativeHelper1407bc6e0(Record->Field28Ref(), Arg5);
                        NativeReleaseSharedRef(TableObj.Object);
                        return true;
                    }
                    if (Cls == 1)
                    {
                        if (NativeHelper1407bc6e0(Record->Field28Ref(), Arg5))
                        {
                            NativeReleaseSharedRef(TableObj.Object);
                            return true;
                        }
                    }
                }
            }
        }
        NativeReleaseSharedRef(TableObj.Object);
    }
    if (NativeByte28(InterfaceThis) != 0)
    {
        *Arg5 = 0;
        return true;
    }
    return false;
}

// ---- Panel slot 0x30 140800860 [dec] ----
// Void. Owner virtual 0x290 -> active row object. Pair selected from Row by mode. If this+0xd8 is valid,
// FUN_1407cac50(FUN_142ee5dd0(this+0xd8), Arg2, &pair-copy, 0). Pair ref released at end.
void NativePanelSlot0x30_140800860(void* InterfaceThis, int32 Arg2, const void* Row)
{
    UCommonActivatablePanel* Owner = NativePanelOwner0x8_1408056ac(InterfaceThis);
    void* Active = NativeHelper14081b480(NativeOwnerVirtual0x290(Owner));
    if (Active == nullptr)
        return;
    FNativeDispatchPair Pair = NativeSelectPairByMode(Row, Active);
    if (NativeHelper142ee7880(static_cast<uint8*>(InterfaceThis) + 0xd8))
    {
        void* Obj = NativeHelper142ee5dd0(static_cast<uint8*>(InterfaceThis) + 0xd8);
        NativeHelper1407cac50(Obj, Arg2, &Pair, 0);
        NativeReleaseTemps140800860();
    }
    NativeReleaseSharedRef(Pair.Object);
}

// ---- Not translated (UNKNOWN) ----
// Global 0x0 1407a5da8 (dtor thunk), panel 0x0 1407a5d84 (dtor thunk).
// Tick bodies 140802440 and 140802200 are out of scope.
// Eligibility 1407d0160 is external (another worker).
