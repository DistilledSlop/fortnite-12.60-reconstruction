// Translation of the cancel chain. NOT COMPILABLE and NOT INTEGRATED: depends on UNKNOWN layouts
// and on the placeholders in declarations.inc. Raw offsets are kept where the pseudo-C shows them.

// 0x1421cb230 (CancelExec). Disassembly: tail-jump to 0x1411ce2d0 when helper != 0.
void UFortMcpContext::CancelEpicPurchasingDisplay()
{
    NativeCancelHelper1421cb230* Helper = NativeCancelHelper378;
    if (Helper == nullptr)
    {
        return;
    }
    NativeCancelBody1411ce2d0(Helper);
}

// 0x1411ce2d0 (CancelBody). Runs only when Helper+0x50 (Impl) is non-null.
void NativeCancelBody1411ce2d0(NativeCancelHelper1421cb230* Helper)
{
    if (*(void**)((uint8*)Helper + 0x50) == nullptr)
    {
        return;
    }
    FString Reason; // zero-initialized like the pseudo-C (lStack_18 = 0, uStack_10 = 0)
    NativeFStringPrepare140680160(&Reason, 0xe);
    int32 NewNum = *(int32*)((uint8*)&Reason + 0x8) + 0xe;
    *(int32*)((uint8*)&Reason + 0x8) = NewNum;
    if (*(int32*)((uint8*)&Reason + 0xc) < NewNum)
    {
        NativeFStringResize1406800e0(&Reason);
    }
    NativeWideCopy142bc7780(*(void**)&Reason, L"UserCancelled", 0x1c);
    NativeCancelInner1411d0c30(Helper, &Reason);
    if (*(void**)&Reason != nullptr)
    {
        NativeFreeData142bee710(*(void**)&Reason);
    }
}

// 0x1411d0c30 (CancelInner). Purchase-flow completion. Helper+0x50 = Impl*.
// Impl+0x50/+0x58 = TSharedPtr<FPurchaseReceipt>-shaped {Object, ReferenceController} (VERIFIED from the
// 0x140d0e7f0 out pair and the stores in the failure path); non-null Object is the "success" test.
void NativeCancelInner1411d0c30(NativeCancelHelper1421cb230* Helper, const FString* Error)
{
    void** HelperImplSlot = (void**)((uint8*)Helper + 0x50);   // plVar1
    uint8* Impl = (uint8*)*HelperImplSlot;                       // lVar9
    bool bSucceeded = false;                                     // uVar11
    uint8 ResultBuf[0x48];          // auStack_58 = FOnlineError (VERIFIED size 0x48; was 0x40, which is too small).
                                    // +0x0 bSucceeded, +0x8 ErrorRaw (FString), +0x18 ErrorCode (FString), +0x28 ErrorMessage (FText), +0x40 Result.
    void* PairCopy[2] = {};         // uStack_88 (Object), plStack_80 (Controller)
    void* Slot78[2] = {};           // auStack_78 out slot for NativeCreateSharedResult140d0e7f0

    if (*(void**)(Impl + 0x50) != nullptr)
    {
        // Success path (goto LAB_1411d0d8e in the pseudo-C).
        if (NativeLogLevel1480eded8 > 4)
        {
            NativeUELog142c54b70(0, 0, NativeLogCategory1480ededc, 5, L"Purchase Flow succeeded");
        }
        bSucceeded = true;
    }
    else
    {
        // Failure path.
        if (NativeLogLevel1480eded8 > 2)
        {
            const wchar_t* Text = (*(int32*)((uint8*)Error + 0x8) == 0) ? NativeEmptyStr1459a4b30 : *(const wchar_t**)Error;
            NativeUELog142c54b70(0, 0, NativeLogCategory1480ededc, 3, L"Purchase Flow failed with error: %s", Text);
            Impl = (uint8*)*HelperImplSlot;
        }
        void** Shared = NativeCreateSharedResult140d0e7f0(*(void**)(Impl + 0x10), Slot78);
        void* NewObj = Shared[0];
        void* NewCtrl = Shared[1];
        if (NewCtrl != nullptr)
        {
            ++*(int32*)((uint8*)NewCtrl + 0x8);     // shared count +1 (pseudo-C addref)
        }
        Impl = (uint8*)*HelperImplSlot;
        if ((void*)(Impl + 0x50) == (void*)PairCopy)
        {
            // pseudo-C: address compare with a stack local; kept literally. Goes to LAB_1411d0d3b.
            NativeCtrlRelease1411d0c30(NewCtrl);
        }
        else
        {
            *(void**)(Impl + 0x50) = NewObj;
            void* OldCtrl = *(void**)(Impl + 0x58);
            if (NewCtrl == OldCtrl)
            {
                NativeCtrlRelease1411d0c30(NewCtrl);    // LAB_1411d0d3b
            }
            else
            {
                *(void**)(Impl + 0x58) = NewCtrl;
                if (OldCtrl != nullptr)
                {
                    NativeCtrlRelease1411d0c30(OldCtrl);
                }
            }
        }
        // LAB_1411d0d62: release plStack_70 (source UNKNOWN).
        NativeCtrlRelease1411d0c30(UNKNOWN_plStack_70);
    }

    // LAB_1411d0d8e (common tail).
    Impl = (uint8*)*HelperImplSlot;
    if (*(int32*)(Impl + 0x38) != 0 && *(void**)(Impl + 0x30) != nullptr)
    {
        // Virtual slot 0x28 (returns bool), called on the object at Impl+0x30.
        void* Obj30 = *(void**)(Impl + 0x30);
        bool bValid = ((bool (*)(void*))((*(void***)Obj30)[5]))(Obj30);
        if (bValid)
        {
            void* Target = (*(int32*)(Impl + 0x38) == 0) ? nullptr : *(void**)(Impl + 0x30);
            // Virtual slot 0x50 on Target, no arguments in the pseudo-C.
            ((void (*)(void*))((*(void***)Target)[10]))(Target);
        }
    }

    NativeResultInit140d5ddf0(ResultBuf, bSucceeded);
    NativeResultSetError140d6ff70(ResultBuf, Error);

    // Copy Impl+0x50 / Impl+0x58 into the call pair, with addref on the controller.
    Impl = (uint8*)*HelperImplSlot;
    PairCopy[0] = *(void**)(Impl + 0x50);
    PairCopy[1] = *(void**)(Impl + 0x58);
    if (PairCopy[1] != nullptr)
    {
        ++*(int32*)((uint8*)PairCopy[1] + 0x8);
        Impl = (uint8*)*HelperImplSlot;
    }
    // Impl+0x40 / Impl+0x48 = FDelegateBase storage (VERIFIED: GetDelegateInstanceProtected returns the
    // allocation only when DelegateSize != 0). Dispatch slot 0x48 is NOT verified (see NOTES.md UNKNOWN 8).
    void* Listener = (*(int32*)(Impl + 0x48) == 0) ? nullptr : *(void**)(Impl + 0x40);
    ((void (*)(void*, void*, void*))((*(void***)Listener)[9]))(Listener, ResultBuf, PairCopy);
    NativeCtrlRelease1411d0c30(PairCopy[1]);

    // Impl pointer on the helper is cleared. Pseudo-C: the check against the stack address
    // (&alStack_68) is always true for a heap-owned helper, so the clear runs.
    *HelperImplSlot = nullptr;
    void* HelperCtrl = *(void**)((uint8*)Helper + 0x58);
    if (HelperCtrl != nullptr)
    {
        *(void**)((uint8*)Helper + 0x58) = nullptr;
        NativeCtrlRelease1411d0c30(HelperCtrl);
    }

    // Release plStack_28 (atomic in the pseudo-C; source UNKNOWN).
    NativeCtrlRelease1411d0c30(UNKNOWN_plStack_28);

    // Free the two result-owned buffers (lStack_40 first, then lStack_50).
    // VERIFIED: lStack_40 = auStack_58+0x18 (ErrorCode data), lStack_50 = auStack_58+0x8 (ErrorRaw data).
    if (*(void**)(ResultBuf + 0x18) != nullptr)
    {
        NativeFreeData142bee710(*(void**)(ResultBuf + 0x18));
    }
    if (*(void**)(ResultBuf + 0x8) != nullptr)
    {
        NativeFreeData142bee710(*(void**)(ResultBuf + 0x8));
    }
}
