// Translation of VA 0x1407d0160 (legacy CommonUI input eligibility) and helper VA 0x1407cf620.
// Control flow follows the asm (image-125.runtime.bin). Address comments are VAs.
// Callee declarations are in declarations.inc. UNKNOWN items are listed in NOTES.md.
// Fragment is not enabled by any policy file.

namespace LegacyEligibilityDetail
{
    // Virtual call helpers. Slot offsets are byte offsets into the vtable (asm uses 0x28 and 0x48).
    inline bool VirtualBool(void* Obj, int32 SlotOffset)
    {
        void** Vtbl = *reinterpret_cast<void***>(Obj);
        return reinterpret_cast<bool (*)(void*)>(Vtbl[SlotOffset / 8])(Obj);
    }

    inline uint8 VirtualByte(void* Obj, int32 SlotOffset)
    {
        void** Vtbl = *reinterpret_cast<void***>(Obj);
        return reinterpret_cast<uint8 (*)(void*)>(Vtbl[SlotOffset / 8])(Obj);
    }

    // 1407d0403-1407d0444 variant: slot 0x48 called with rdx = out byte pointer.
    inline uint8 VirtualByteOut(void* Obj, int32 SlotOffset)
    {
        void** Vtbl = *reinterpret_cast<void***>(Obj);
        uint8 Out = 0;
        reinterpret_cast<void (*)(void*, uint8*)>(Vtbl[SlotOffset / 8])(Obj, &Out);
        return Out;
    }

    // Name temp -> wide string or empty. asm: cmpl $0,0x8(temp); cmovne temp.data (1407d0499-1407d04b1).
    inline const wchar_t* StrOrEmpty(const LegacyFStringTemp& Temp)
    {
        return Temp.Num != 0 ? Temp.Data : LegacyEmptyWideString;
    }

    // Log helper for the record-name lines. Each call site in asm passes
    // (0, 0, &category, 6, fmt, args...). Verbosity 6 is the value compared against
    // the category byte at image 0x80d1df0.
    inline void LogGlobalInput(const wchar_t* Fmt, const wchar_t* Arg1, const wchar_t* Arg2, int32 NumArgs)
    {
        NativeLog142c54b70(0, 0, LegacyGlobalInputLogCategory, 6, Fmt, Arg1, Arg2, NumArgs);
    }

    // Name temp used as log argument: record name = Lookup(Row)->+0x18 via 142ce9f90.
    inline void RecordName(void* RowInputRecord, LegacyFStringTemp& Out)
    {
        void* Record = NativeRecordLookup142ee5dd0(RowInputRecord);
        uint64 NameValue = *reinterpret_cast<uint64*>(static_cast<uint8*>(Record) + 0x18);
        NativeNameValueToString142ce9f90(&NameValue, &Out);
    }
}

// 1407cf620. Asm 1407cf620-1407cf670.
bool NativeHelper1407cf620(void* Widget)
{
    // 1407cf62d: call 143d60da0 first; result kept in bl.
    const uint8 InputType = NativeInputTypeQuery143d60da0(Widget);
    // 1407cf638: GetIsEnabled.
    if (!NativeGetIsEnabled143d5cb70(Widget))
    {
        return false; // 1407cf664
    }
    // 1407cf641-1407cf64e: (byte @ +0x2a1) & 6 must equal 6.
    if ((*(static_cast<uint8*>(Widget) + 0x2a1) & 0x6) != 0x6)
    {
        return false;
    }
    // 1407cf650-1407cf655: dec bl; cmp bl,1; jbe -> return 0 when (byte)(type-1) <= 1.
    if (static_cast<uint8>(InputType - 1) <= 1)
    {
        return false;
    }
    return true; // 1407cf657
}

// 143d5cb70. Asm 3d5cb70-3d5cbf0 (structure only; see declarations.inc note).
bool NativeGetIsEnabled143d5cb70(void* Widget)
{
    LegacySharedPtrRaw Sret;
    NativeGetCachedWidget143d5a140(Widget, &Sret);                              // 3d5cb7a-3d5cb82 (return value unused)
    void* SlateWidget = Sret.Object;                                            // 3d5cb87 (rbx = [rsp+0x20])
    uint8 Result;
    if (SlateWidget != nullptr)                                                 // 3d5cb8c-3d5cb8f
    {
        uint8* Attr = static_cast<uint8*>(SlateWidget) + 0x190;
        // 3d5cb90-3d5cbb9: qword [+0x1a0] test, then ptr [+0x198]->vtbl[0x28].
        bool bLive = *reinterpret_cast<int64*>(static_cast<uint8*>(SlateWidget) + 0x1a0) != 0;
        if (bLive)
        {
            void* P = *reinterpret_cast<void**>(static_cast<uint8*>(SlateWidget) + 0x198);
            bLive = (P != nullptr) && LegacyEligibilityDetail::VirtualBool(P, 0x28);
        }
        if (bLive)
        {
            // 3d5cbb0-3d5cbc7: dword [+0x1a0] test selects ptr [+0x198] or null, then vtbl[0x48].
            void* P = (*reinterpret_cast<int32*>(static_cast<uint8*>(SlateWidget) + 0x1a0) != 0)
                ? *reinterpret_cast<void**>(static_cast<uint8*>(SlateWidget) + 0x198)
                : nullptr;
            *Attr = LegacyEligibilityDetail::VirtualByte(P, 0x48);             // 3d5cbca
        }
        Result = *Attr;                                                         // 3d5cbd0
    }
    else
    {
        // 3d5cbd9-3d5cbe3: bIsEnabled bit = bit 2 of byte @ +0xb4, masked to one bit.
        Result = static_cast<uint8>((*(static_cast<uint8*>(Widget) + 0xb4) >> 2) & 1);
    }
    // 3d5cbe6-3d5cc13: release the out pair's control block if non-null.
    NativeReleaseSharedControl(Sret.Ctrl);
    return Result != 0;
}

// 1407d0160. Asm 1407d0160-1407d0657.
bool NativeEligibility1407d0160(void* UnusedRcx, void* RowInputRecord)
{
    (void)UnusedRcx; // rcx is overwritten at 1407d0185 before any read.

    // 1407d018b-1407d019a: GetIsEnabled on the record's root.
    void* Root = NativeRecordLookup142ee5dd0(RowInputRecord);
    if (!NativeGetIsEnabled143d5cb70(Root))
    {
        // 1407d019c-1407d01a3: verbosity gate.
        if (LegacyGlobalInputLogVerbosity < 6)
        {
            return false; // 1407d028b
        }
        // 1407d01a9-1407d01d0 / 1407d024a-1407d028b: "Not Enabled" line.
        LegacyFStringTemp Name;
        LegacyEligibilityDetail::RecordName(RowInputRecord, Name);
        LegacyEligibilityDetail::LogGlobalInput(LegacyFmtNotEnabled, LegacyEligibilityDetail::StrOrEmpty(Name), nullptr, 1);
        if (Name.Data != nullptr)
        {
            NativeFree142bee710(const_cast<wchar_t*>(Name.Data));
        }
        return false; // 1407d028b
    }

    // 1407d01d2-1407d01e0: enabled path. Owner = lookup(row).
    void* Owner = NativeRecordLookup142ee5dd0(RowInputRecord);
    if (Owner != nullptr)
    {
        // 1407d01e6-1407d0208: Ctx = 140807cd0(); require idx <= owner->list->count and list[idx] == Ctx+0x30.
        void* Ctx = NativeContext140807cd0();
        uint8* OwnerU8 = static_cast<uint8*>(Owner);
        uint8* OwnerList = *reinterpret_cast<uint8**>(OwnerU8 + 0x10);     // 1407d01eb
        int32 Index = *reinterpret_cast<int32*>(static_cast<uint8*>(Ctx) + 0x38); // 1407d01f3 movslq
        bool bIndexOk = !(Index > *reinterpret_cast<int32*>(OwnerList + 0x38));    // 1407d01f7-1407d01fa (jg)
        bool bPtrOk = false;
        if (bIndexOk)
        {
            void** Array = *reinterpret_cast<void***>(OwnerList + 0x30);     // 1407d0200
            bPtrOk = (Array[Index] == static_cast<uint8*>(Ctx) + 0x30);      // 1407d0204-1407d0208 (jne)
        }
        // 1407d0211-1407d0218: helper1407cf620 must be false for the log branch.
        if (bIndexOk && bPtrOk && !NativeHelper1407cf620(Owner))
        {
            // 1407d021a-1407d0221: verbosity gate.
            if (LegacyGlobalInputLogVerbosity < 6)
            {
                return false; // 1407d028b
            }
            // 1407d0223-1407d028b: "Interaction Not Enabled" line.
            LegacyFStringTemp Name;
            LegacyEligibilityDetail::RecordName(RowInputRecord, Name);
            LegacyEligibilityDetail::LogGlobalInput(LegacyFmtInteractionNotEnabled, LegacyEligibilityDetail::StrOrEmpty(Name), nullptr, 1);
            if (Name.Data != nullptr)
            {
                NativeFree142bee710(const_cast<wchar_t*>(Name.Data));
            }
            return false;
        }
    }

    // 1407d0292: general path. Only reached when no early return above.
    LegacyLocalPath Path;
    NativeLocalPathCtor142f44d90(&Path);                                    // 1407d02ae (rcx = auStack_198)
    uint64 Global = LegacyGlobalSlateObjectRef;                            // 1407d02b3 (rdi = [8271318])
    void* Row2 = NativeRecordLookup142ee5dd0(RowInputRecord);              // 1407d02bd
    LegacySharedPtrRaw Sret;
    LegacySharedPtrRaw* Ret = NativeGetCachedWidget143d5a140(Row2, &Sret);  // 1407d02c2-1407d02ca
    LegacySharedPtrRaw Pinned;
    Pinned.Object = Ret->Object;                                            // 1407d02cf (rcx = [rax])
    Pinned.Ctrl = Ret->Ctrl;                                                // 1407d02d2 (rsi = [rax+8])
    Ret->Object = nullptr;                                                  // 1407d02de
    Ret->Ctrl = nullptr;                                                    // 1407d02d6
    // 1407d02e5-1407d0318: release the control stored at rsp+0x68 (Sret.Ctrl). Relation to Ret UNKNOWN.
    NativeReleaseSharedControl(Sret.Ctrl);

    void* PinnedCtrlForCleanup = Pinned.Ctrl;                               // rsi kept for 1407d05f0 release
    NativeAddRefSharedControl(Pinned.Ctrl);                                 // 1407d0326-1407d032b

    LegacySharedPtrRaw Saved = Pinned;                                      // 1407d031b movaps/xmm6 save
    bool bFound = NativeFindPath14300d060(Global, &Pinned, &Path, LegacyGlobalInputPathFlag); // 1407d032e-1407d0342
    bool bResult = false;

    if (!bFound)
    {
        // 1407d057d-1407d05ee: "No Path To Widget" line, then return false via cleanup.
        if (LegacyGlobalInputLogVerbosity >= 6)
        {
            LegacyFStringTemp Name;
            LegacyEligibilityDetail::RecordName(RowInputRecord, Name);
            LegacyEligibilityDetail::LogGlobalInput(LegacyFmtNoPathToWidget, LegacyEligibilityDetail::StrOrEmpty(Name), nullptr, 1);
            if (Name.Data != nullptr)
            {
                NativeFree142bee710(const_cast<wchar_t*>(Name.Data));
            }
        }
    }
    else
    {
        // 1407d034f-1407d035a: restore saved pair and AddRef again.
        Pinned = Saved;
        NativeAddRefSharedControl(Pinned.Ctrl);
        // 1407d035d-1407d0366: FindOwner(path, &pinned).
        if (NativeFindOwner142f4c5b0(&Path, &Pinned))
        {
            // 1407d0373-1407d0394: element base and end.
            uint8* Base = Path.Raw + 0x8;
            void* HeapData = *reinterpret_cast<void**>(Path.Raw + 0x128);
            if (HeapData != nullptr)
            {
                Base = static_cast<uint8*>(HeapData);
            }
            int32 Count = *reinterpret_cast<int32*>(Path.Raw + 0x130);
            uint8* End = Base + static_cast<int64>(Count) * LegacyPathElementStride;

            // 1407d039c-1407d0457: element loop. Completing the loop sets bResult = true (7d0465).
            bool bLoopCompleted = true;
            for (uint8* Elem = Base; Elem != End; Elem += LegacyPathElementStride)
            {
                void* Widget = *reinterpret_cast<void**>(Elem + LegacyPathElementWidgetOffset); // 1407d03b0
                uint8* Attr1 = static_cast<uint8*>(Widget) + 0x190;                                // 1407d03b4

                // 1407d03bb-1407d03d2: live if [+0x1a0]!=0, ptr [+0x198]!=0, and ptr->vtbl[0x28]().
                bool bLive1 = false;
                if (*reinterpret_cast<int32*>(Attr1 + 0x10) != 0)
                {
                    void* P = *reinterpret_cast<void**>(Attr1 + 0x8);
                    bLive1 = (P != nullptr) && LegacyEligibilityDetail::VirtualBool(P, 0x28);
                }
                if (bLive1)
                {
                    // 1407d03de-1407d03f2: ptr from [+0x198] if count!=0; *Attr1 = vtbl[0x48]().
                    void* P = (*reinterpret_cast<int32*>(Attr1 + 0x10) == 0)
                        ? nullptr
                        : *reinterpret_cast<void**>(Attr1 + 0x8);
                    *Attr1 = LegacyEligibilityDetail::VirtualByte(P, 0x48);
                }
                if (*Attr1 == 0)
                {
                    // 1407d04ce-1407d0571: "!IsEnabled" line, then break (bResult = false).
                    if (LegacyGlobalInputLogVerbosity >= 6)
                    {
                        LegacyFStringTemp RecName;
                        LegacyEligibilityDetail::RecordName(RowInputRecord, RecName);
                        LegacyFStringTemp WidgetName;
                        NativeWidgetName142fc3fe0(&WidgetName, Widget);
                        LegacyEligibilityDetail::LogGlobalInput(
                            LegacyFmtNotIsEnabled,
                            LegacyEligibilityDetail::StrOrEmpty(WidgetName),
                            LegacyEligibilityDetail::StrOrEmpty(RecName),
                            2);
                        if (WidgetName.Data != nullptr)
                        {
                            NativeFree142bee710(const_cast<wchar_t*>(WidgetName.Data));
                        }
                        if (RecName.Data != nullptr)
                        {
                            NativeFree142bee710(const_cast<wchar_t*>(RecName.Data));
                        }
                    }
                    bLoopCompleted = false;
                    break;
                }

                // 1407d03fd-1407d044b: attr2 at +0x1a8, live if [+0x1b8]!=0, ptr [+0x1b0]!=0, vtbl[0x28]().
                uint8* Attr2 = static_cast<uint8*>(Widget) + 0x1a8;
                bool bLive2 = false;
                if (*reinterpret_cast<int32*>(Attr2 + 0x10) != 0)
                {
                    void* P = *reinterpret_cast<void**>(Attr2 + 0x8);
                    bLive2 = (P != nullptr) && LegacyEligibilityDetail::VirtualBool(P, 0x28);
                }
                if (bLive2)
                {
                    void* P = (*reinterpret_cast<int32*>(Attr2 + 0x10) == 0)
                        ? nullptr
                        : *reinterpret_cast<void**>(Attr2 + 0x8);
                    *Attr2 = LegacyEligibilityDetail::VirtualByteOut(P, 0x48);   // 1407d0439-1407d0449
                }
                if ((*Attr2 & 0x10) == 0)
                {
                    // 1407d046c-1407d0571: "!AreChildrenHitTestVisible" line, then break.
                    if (LegacyGlobalInputLogVerbosity >= 6)
                    {
                        LegacyFStringTemp RecName;
                        LegacyEligibilityDetail::RecordName(RowInputRecord, RecName);
                        LegacyFStringTemp WidgetName;
                        NativeWidgetName142fc3fe0(&WidgetName, Widget);
                        LegacyEligibilityDetail::LogGlobalInput(
                            LegacyFmtNotChildrenHitTestVisible,
                            LegacyEligibilityDetail::StrOrEmpty(WidgetName),
                            LegacyEligibilityDetail::StrOrEmpty(RecName),
                            2);
                        if (WidgetName.Data != nullptr)
                        {
                            NativeFree142bee710(const_cast<wchar_t*>(WidgetName.Data));
                        }
                        if (RecName.Data != nullptr)
                        {
                            NativeFree142bee710(const_cast<wchar_t*>(RecName.Data));
                        }
                    }
                    bLoopCompleted = false;
                    break;
                }
            }
            bResult = bLoopCompleted; // 1407d045d (1) or 1407d0571 path (0)
        }
    }

    // 1407d05f0-1407d0629: common cleanup for the general path.
    NativeReleaseSharedControl(PinnedCtrlForCleanup);
    NativeLocalPathDtor14079aa60(&Path);
    return bResult; // 1407d063a: movzbl bl
}
