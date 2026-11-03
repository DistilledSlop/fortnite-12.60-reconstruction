// restart_modal_translation_runtime: definitions translated from porting/generated/decomp-imports/Imports/*.c.
// Fragment only. Not compiled. Each statement maps to pseudo-C in that file; UNKNOWN items are listed in NOTES.md.

// ---- Local helper: FName-derived text, "" when the FString is empty (count @+8 == 0) ----
const wchar_t* NativeNameText(uint64_t NameIn)
{
    void* OutBuf = nullptr;
    FString* Str = static_cast<FString*>(NativeCall142ce9f90(&NameIn, &OutBuf));
    const wchar_t* Text = (Str->ArrayNum == 0) ? NativeEmptyString1459a4b30 : Str->Data;
    if (OutBuf) NativeCall142bee710(OutBuf);
    return Text;
}

// ---- UFortRestartClientModal::HandleButtonClicked (RestartClick__144f5f9f0.c, VA 0x144f5f9f0) ----
void UFortRestartClientModal::HandleButtonClicked()
{
    NativeRestartPlatform141ed81a0(1, bNativeRestartFlag448);     // (char)param_1[0x89] = byte +0x448 (verified)
    if (!bNativeStackMembership304)                               // byte +0x304 (verified cmpb/je)
    {
        NativeCall1407b7ed0(this);                                // tail jmp 0x7b7ed0, rcx = this (verified)
        return;
    }
    NativeVtblCall(this, 0x4c0);                                  // tail jmp *0x4c0(vtable); target NativePanelPop1407e1130
}

// ---- NativeRestartPlatform141ed81a0 (RestartPlatform__141ed81a0.c, VA 0x141ed81a0) ----
void NativeRestartPlatform141ed81a0(char Mode, char bExitFlag)
{
    if (Mode == 0)
    {
        // Launcher branch. Not reached from HandleButtonClicked (Mode == 1).
        uint64_t NameTmp = 0;
        uint64_t* FNameIdx = NativeCall142cd49c0(&NameTmp, "LauncherCheck", 1);
        uint64_t Ctx = NativeCall142cbd480();                       // thunk_FUN_142cbd480, UNKNOWN
        void* Launcher = NativeCall142cc0600(Ctx, *FNameIdx);      // thunk_FUN_142cc0600, UNKNOWN
        uint64_t Params[2] = {0, 0};                               // uStack_18 / uStack_10
        NativeVtblCall(Launcher, 0x48, 1, &Params);                // vtable +0x48, args (1, &Params)
        if (NativeGlobalLogVerbosity1481348e0 > 3)
        {
            NativeLog142c54b70(0, 0, &NativeLogCategory1481348e4, 4,
                L"HandleClientUpdateRequired: RunLauncher(ELauncherAction::AppUpdateCheck)");
        }
    }
    else
    {
        if (Mode != 1)
        {
            return;
        }
        if (NativeCall142bcb050())                                  // platform restart check; nonzero = return
        {
            return;
        }
    }
    if (bExitFlag == 0)
    {
        return;
    }
    NativeCall142ce8ac0(0);
}

// ---- UCommonActivatablePanel::NativePanelPop1407e1130 (PanelPop__1407e1130.c, VA 0x1407e1130; vtable +0x4c0) ----
void UCommonActivatablePanel::NativePanelPop1407e1130()
{
    if (NativeCall142ee7880(&NativeInputManagerWeak3B0))           // IsValid on weak @+0x3b0
    {
        // Get on the weak pointer. The binary has no null check here (BEHAVIOR.md).
        UCommonInputManager* Manager = static_cast<UCommonInputManager*>(NativeCall142ee5dd0(&NativeInputManagerWeak3B0));
        Manager->NativePopActivatable1407e0c80(this);
    }
}

// ---- UCommonInputManager::NativePopActivatable1407e0c80 (PopActivatable__1407e0c80.c, VA 0x1407e0c80) ----
void UCommonInputManager::NativePopActivatable1407e0c80(UCommonActivatablePanel* ActivatablePanel)
{
    if (NativeGlobalLogVerbosity1480d1cd8 > 6)
    {
        NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 7, L"PopActivatablePanel: begin");
    }

    TArray<UCommonActivatablePanel*> Predicted;                    // plStack_a8 / iStack_a0
    NativeCall1407ad3f0(this, &Predicted);

    if (ActivatablePanel != nullptr)
    {
        // cVar10 in pseudo-C: 1 when the panel is NOT in ActivatablePanelStack (+0xb8, count +0xc0).
        bool bAbsentCommitted = !ActivatablePanelStack.Contains(ActivatablePanel);
        // cVar8 in pseudo-C: 1 when the panel is NOT in the predicted stack.
        bool bAbsentPredicted = !Predicted.Contains(ActivatablePanel);

        if (bAbsentCommitted != bAbsentPredicted && NativeGlobalLogVerbosity1480d1cd8 > 5)
        {
            NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                L"PopActivatablePanel: Redundant pop detection mismatch - queueing pop for panel \"%s\" was %s in legacy, is %s now.",
                NativeNameText(ActivatablePanel->NativeField18),       // +0x18 identity UNKNOWN (NOTES.md)
                bAbsentCommitted ? L"redundant" : L"NOT redundant",
                bAbsentPredicted ? L"redundant" : L"NOT redundant");
        }

        if (!bAbsentPredicted)
        {
            // Pop path. Walk the predicted stack top-down; stop at ActivatablePanel.
            int32_t I = Predicted.Num() - 1;                        // iVar12
            for (; I >= 0; --I)
            {
                UCommonActivatablePanel* Entry = Predicted[I];     // lVar9
                uint32_t Payload[4];
                NativeCall142c9ff10(Payload);                      // per-entry 16-byte payload, UNKNOWN meaning
                if (NativeGlobalLogVerbosity1480d1cd8 > 5)
                {
                    FString Desc;
                    NativeCall142ca88a0(Payload, &Desc, 0);
                    NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                        L"PopActivatablePanel: Adding pop op (%s) for panel: %s",
                        Desc.ArrayNum == 0 ? NativeEmptyString1459a4b30 : Desc.Data,
                        Entry == nullptr ? NativeEmptyString1459a4b30 : NativeNameText(Entry->NativeField18));
                }
                FNativeOperationRaw Op = {};
                Op.Operation = 3;                                  // Pop (verified)
                Op.Panel = Entry;
                Op.NativeOpFlag10 = 1;
                Op.NativeOpFlag11 = 1;
                Op.NativeOpFlag12 = 1;
                for (int k = 0; k < 4; ++k) Op.NativeOpPayload14[k] = Payload[k];
                NativeCall1407a95e0(this, &Op);                    // return value ignored by pseudo-C
                if (Entry == ActivatablePanel)                     // compares the loop entry, not the helper return
                {
                    break;
                }
            }

            // After the loop, I is the index of ActivatablePanel, or -1 if the loop ran out.
            if (I > 0)
            {
                UCommonActivatablePanel* Prev = Predicted[I - 1];  // lVar11
                uint32_t Payload[4];
                NativeCall142c9ff10(Payload);
                if (NativeGlobalLogVerbosity1480d1cd8 > 5)
                {
                    FString Desc;
                    NativeCall142ca88a0(Payload, &Desc, 0);
                    NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                        L"PopActivatablePanel: Adding intro op (%s) to previous panel: %s",
                        Desc.ArrayNum == 0 ? NativeEmptyString1459a4b30 : Desc.Data,
                        Prev == nullptr ? NativeEmptyString1459a4b30 : NativeNameText(Prev->NativeField18));
                }
                int32_t Slot = Operations.Num();                   // +0xf0, iVar12 read again in the binary
                if (Operations.Max() < Slot + 1)                   // +0xf4
                {
                    NativeGrow14069ea30(&Operations, Slot);
                }
                FNativeOperationRaw* Elem = reinterpret_cast<FNativeOperationRaw*>(Operations.GetData()) + Slot;
                Elem->Operation = 0;                               // Intro (verified)
                Elem->Panel = Prev;
                Elem->NativeOpFlag10 = 1;
                Elem->NativeOpFlag11 = 1;
                Elem->NativeOpFlag12 = 1;
                for (int k = 0; k < 4; ++k) Elem->NativeOpPayload14[k] = Payload[k];
                // Count write *(int*)(this+0xf0) = Slot + 1 is UNKNOWN API for TArray here (NOTES.md).
            }

            if (!bNativeFlagE0)                                    // byte +0xe0 (verified read)
            {
                NativeCall1407ab920(this);
            }
        }
        else if (NativeGlobalLogVerbosity1480d1cd8 > 5)
        {
            // Text is "InternalPushActivatablePanel" in the binary although this is the pop function (kept verbatim).
            NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                L"InternalPushActivatablePanel: Redundant pop detected - panel \"%s\" will already not be on the stack if queued to pop.",
                NativeNameText(ActivatablePanel->NativeField18));
        }
    }

    if (NativeGlobalLogVerbosity1480d1cd8 > 6)
    {
        NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 7, L"PopActivatablePanel: end");
    }
}

// ---- UCommonActivatablePanel::NativePanelOutroReq1407ce810 (PanelOutroReq__1407ce810.c, VA 0x1407ce810; vtable +0x4a8) ----
void UCommonActivatablePanel::NativePanelOutroReq1407ce810()
{
    if (!bNativeFlag303)                                           // byte +0x303 (verified cmpb/je)
    {
        return;
    }
    NativeHelper1407ae640();                                       // Helper_ae640
    NativeHelper140809130();                                       // Helper_809130
    NativeLog142cc40c0(NativeGlobal1482675e8, L"Deactivate: %s", NativeNameText(NativeField18));  // format verified
}

// ---- UCommonActivatablePanel::NativeHelper1407ae640 (Helper_ae640__1407ae640.c, VA 0x1407ae640) ----
void UCommonActivatablePanel::NativeHelper1407ae640()
{
    if (!NativeCall142ee7880(&NativeInputManagerWeak3B0))          // IsValid guard
    {
        return;
    }
    // pseudo-C: (ptr3C0 != 0) || ((int3CC == 0) & bVar2) == 0, with bVar2 = FUN_142cd8ed0(int3C8, 0) evaluated in that clause.
    bool bVar2 = (NativePtr3C0 != nullptr) ? false : NativeCall142cd8ed0(NativeInt3C8, 0);
    if (((NativePtr3C0 != nullptr) || !((NativeInt3CC == 0) && bVar2)) &&
        (NativeFloat308 > 0.0f))
    {
        int32_t Idx[2] = {0, 0};                                   // aiStackX_8
        NativeCall1407bdba0(&NativeArray360, Idx, &NativePtr3C0);  // returns index in Idx[0]
        uint8_t* Elem = (Idx[0] == -1) ? nullptr : reinterpret_cast<uint8_t*>(NativeArray360.GetData()) + Idx[0] * 0x98;
        uint8_t* Sub = (Elem == nullptr) ? nullptr : Elem + 0x10;  // lVar4 = elem + 0x10, 0 when elem == 0
        if (Sub != nullptr)
        {
            NativeCall1407ac590(Sub, 0);
        }
    }
    NativeFloat308 = 0.0f;                                         // dword +0x308 = 0
    NativePtr3C0 = nullptr;                                        // +0x3c0..+0x3cf zeroed (16 bytes)
    NativeInt3C4 = 0;
    NativeInt3C8 = 0;
    NativeInt3CC = 0;
    uint8_t* Mgr = static_cast<uint8_t*>(NativeCall142ee5dd0(&NativeInputManagerWeak3B0));  // Get, no null check
    *reinterpret_cast<uint64_t*>(Mgr + 0xa8) = 0;
    *reinterpret_cast<uint64_t*>(Mgr + 0xb0) = 0;
}

// ---- UCommonActivatablePanel::NativeHelper140809130 (Helper_809130__140809130.c, VA 0x140809130) ----
void UCommonActivatablePanel::NativeHelper140809130()
{
    uint64_t Arg = NativeCall142ec7730(this, NativeGlobal1480d2260);
    NativeVtblCall(this, 0x210, Arg, uint64_t(0));                 // tail jmp *0x210(vtable), r8 = 0
}

// ---- UCommonActivatablePanel::NativeOutroDefault1407bb630 (OutroDefault__1407bb630.c, VA 0x1407bb630; vtable +0x4b0) ----
// The entry RDX is forwarded to the tail call (BEHAVIOR.md section 2). Its type is UNKNOWN and not in the pseudo-C signature.
void UCommonActivatablePanel::NativeOutroDefault1407bb630()
{
    bNativeFlag303 = false;                                        // movb $0,0x303(%rbx) (BEHAVIOR.md)
    NativeVtblCall(this, 0x4d8);                                   // target stub 0x1406801c0, ret 0 (BEHAVIOR.md 2.1)
    bIsActive = false;                                             // byte +0x288 (pseudo-C param_1+0x51)
    NativeVtblCall(this, 0x490 /* + forwarded RDX, UNKNOWN */);    // tail jmp; target 0x145160190 partly described
}

// ---- NativePushWriter1407fc713 (PushWriter__1407fc713.c, VA 0x1407fc713). FRAGMENT: entry is mid-function. ----
// Parameters are the entry registers/locals the pseudo-C reads without setting. Their identity is UNKNOWN (NOTES.md).
//   ObjRbx        = RBX. Verified: movb $1,0x304(%rbx) at 0x7fc713 and mov %rbx,%rcx before call 0x8090b0.
//   ObjR14        = R14 (lea 0x48(%r14) at 0x7fc728, verified).
//   PanelRbpM31   = [RBP-0x31]. Used as the panel (+0x3b0, +0x3d0, +0x3e0, +0x288, vcall +0x488). Identity UNKNOWN.
//   RdiOpArg      = RDI low byte (cVar6). Op code, UNKNOWN.
//   OpByte28/29   = [RBP-0x28] / [RBP-0x29] op bytes, UNKNOWN source.
//   PayloadRbpM25 = 16 bytes at [RBP-0x25..-0x19] copied into the staging block. UNKNOWN source.
void NativePushWriter1407fc713(void* ObjRbx, void* ObjR14, UCommonActivatablePanel* PanelRbpM31,
                               uint8_t RdiOpArg, uint8_t OpByte28, uint8_t OpByte29, const uint32_t PayloadRbpM25[4])
{
    *(static_cast<uint8_t*>(ObjRbx) + 0x304) = 1;                  // bNativeStackMembership304 = true (verified)
    NativeVtblCall(ObjRbx /* UNKNOWN: RCX at 0x7fc71a, not set in window */, 0x4e8);  // slot +0x4e8, stub 0x1406801c0
    NativeCall1408090b0(ObjRbx);                                   // mov %rbx,%rcx; call 0x8090b0 (verified)
    NativeCall140684de0(static_cast<uint8_t*>(ObjR14) + 0x48);     // lea 0x48(%r14),%rcx; call (verified)

    const uint8_t OpByte = RdiOpArg;                               // cVar6
    if (NativeGlobalLogVerbosity1480d1cd8 > 5)
    {
        NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
            L"StartNextOperation: activatable panel pushed on stack for panel: %s (%s)",
            NativeNameText(PanelRbpM31 ? PanelRbpM31->NativeField18 : 0), NativeEmptyString1459a4b30);
    }

    if (OpByte28 == OpByte)
    {
        if (NativeGlobalLogVerbosity1480d1cd8 > 5)
        {
            NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                L"StartNextOperation: EOperation::Push without intro or activation, calling StartNextOperation");
        }
        NativeCall1407fc000();
    }
    else
    {
        // Staging block at [RBP-0x59]: function pointer, then 4 dwords from PayloadRbpM25 at [RBP-0x9].
        uint64_t RecordA = reinterpret_cast<uint64_t>(NativeFn1407dc080);     // *(code**)(RBP-0x59)
        uint32_t WordsA[4] = {PayloadRbpM25[0], PayloadRbpM25[1], PayloadRbpM25[2], PayloadRbpM25[3]};
        NativeCall140786850(&PanelRbpM31->NativeCallback3D0, static_cast<uint32_t>(RecordA),
                            &RecordA, WordsA);                                       // bind (UNKNOWN semantics)
        uint64_t RecordB = reinterpret_cast<uint64_t>(NativeFn1407dbc80);     // second staging write
        NativeCall140786850(&PanelRbpM31->NativeCallback3E0, static_cast<uint32_t>(RecordB),
                            &RecordB, WordsA);                                       // bind (UNKNOWN semantics)

        // Label is "begin intro" when OpByte29 differs from OpByte, else "activate".
        const char* Label = (OpByte29 != OpByte) ? "begin intro" : "activate";
        if (Label[0] != OpByte)
        {
            int64_t K = -1;
            do { ++K; } while (Label[K] != OpByte);
            int32_t Len = static_cast<int32_t>(K) + 1;             // iVar3
            // Scratch-buffer build with NativeCall140680160 / 1406800e0 / 140789600 (semantics UNKNOWN):
            // NativeCall140789600(Buf, Len, Label, Len, 0x3f). Buffer setup not reproduced (NOTES.md).
            (void)Len;
        }

        if (NativeGlobalLogVerbosity1480d1cd8 > 5)
        {
            NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 6,
                L"StartNextOperation: %s (%s) for panel: %s",
                NativeEmptyString1459a4b30, NativeEmptyString1459a4b30,   // op text and label text: UNKNOWN (NOTES.md)
                NativeNameText(PanelRbpM31->NativeField18));
        }

        NativeCall142ee3b30(&PanelRbpM31->NativeInputManagerWeak3B0);      // weak op on panel +0x3b0

        if (OpByte29 == 0)
        {
            PanelRbpM31->bIsActive = true;                          // byte +0x288 = 1 (pseudo-C)
            NativeVtblCall(PanelRbpM31, 0x488);                     // vcall +0x488, no arguments
        }
        else
        {
            PanelRbpM31->bNativeFlag303 = true;                     // byte +0x303 = 1
            NativeCall1408090f0();
        }
    }

    if (NativeGlobalLogVerbosity1480d1cd8 > 6)
    {
        NativeLog142c54b70(0, 0, &NativeLogCategory1480d1cdc, 7, L"End StartNextOperation");
    }
}
