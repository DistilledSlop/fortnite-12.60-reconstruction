// Legacy input tick lifecycle: definitions (translated from original asm; see NOTES.md).
// Receiver offsets are the asm offsets relative to the receiver the function actually receives.
// Manager-relative offsets are shown in comments (receiver = manager+0x28 for the tick).

// Manager Tick 1407ffc50. Receiver = manager+0x28 (secondary FTickable table slot 0).
// The queue's NativeTickOperationSuspension is inlined here (the asm has no separate call).
void UCommonInputManager_Tick_1407ffc50(void* Self, float DeltaTime)
{
    if (At<uint8>(Self, 0xD5) != 0)                    // bNativeOperationInputSuspended (manager+0xFD)
    {
        if (At<uint8>(Self, 0xD4) != 0) return;        // bNativeProcessingOperations (manager+0xFC)
        if (At<int32>(Self, 0xC8) != 0) return;        // Operations.Num (manager+0xF0)
        if (++At<int32>(Self, 0xD8) <= 2) return;      // NativeIdleOperationTicks (manager+0x100)
        At<uint8>(Self, 0xD5) = 0;
        // Byte at VA 0x1480d1cd8 >= 6 selects a verbose log (text/category UNKNOWN; not in the queue fragment; NOTES D1).
        if (*reinterpret_cast<uint8*>(0x1480d1cd8) >= 6) Unknown_142c54b70();
        Unknown_1406934e0(At<void*>(Self, 0x68), false); // NativeInputSuspensionChanged (manager+0x90).Broadcast(false)
        return;
    }
    void* Obj = At<void*>(Self, 0x80);                 // manager+0xA8: TScriptInterface-shaped object word
    void* Iface = At<void*>(Self, 0x88);               // manager+0xB0: interface word
    if (!Obj || !Iface) return;
    VSlot<void (*)(void*, float)>(Iface, 0x28)(Iface, DeltaTime); // held tick = vtable slot 0x28 (not 0x30)
}

// Panel held tick 140802200 (23b). Receiver = interface at panel+0x2d8.
void PanelHeldTick_140802200(void* Self, float DeltaTime)
{
    void* Obj = static_cast<uint8*>(Self) - 0x2d8;     // r15 in asm

    // Gate: row handle at Self+0xE8 (DataTable +0xE8, RowName.Index +0xF0, RowName.Number +0xF4).
    if (At<void*>(Self, 0xE8) == nullptr && Unknown_142cd8ed0(At<int32>(Self, 0xF0), 0)
        && At<int32>(Self, 0xF4) == 0) return;

    At<float>(Self, 0x30) += DeltaTime;                // held duration (panel+0x308)

    void* R = Unknown_14078c640(static_cast<uint8*>(Self) + 0xE8, reinterpret_cast<void*>(kRowTableVA_1459fa130)); // rbx
    void* Input1 = VSlot<void* (*)(void*)>(Obj, 0x290)(Obj); // obj vtable slot 0x290
    void* Input = Unknown_14081b480(Input1);             // r14
    if (Input == nullptr) return;

    Unknown_14081b540(Input);                          // first call result discarded (asm 0x1408022a3)
    uint8 B = Unknown_14081b540(Input);                // type selector (full byte; asm movzbl)
    uint8 C = Unknown_14081b530(Input);                // index used only when B == 1

    uint8* pE;   // enabled byte
    float* pT;   // hold time
    uint8* RB = static_cast<uint8*>(R);
    if (B == 1)      { pT = reinterpret_cast<float*>(RB + 0xFC + C * 0xA8); pE = RB + 0xF9 + C * 0xA8; }
    else if (B == 2) { pT = reinterpret_cast<float*>(RB + 0x39C);           pE = RB + 0x399; }
    else             { pT = reinterpret_cast<float*>(RB + 0x54);            pE = RB + 0x51; }
    if (*pE == 0) return;                              // hold enabled
    float HoldTime = *pT;

    int32 Idx = 0;
    Unknown_1407bdba0(static_cast<uint8*>(Self) + 0x88, &Idx, static_cast<uint8*>(Self) + 0xE8);
    uint8* E = nullptr;                                // entry = record base + 0x10
    if (Idx != -1)
    {
        uint8* Base = At<uint8*>(Self, 0x88);
        uint8* Rec = Base + static_cast<int64>(Idx) * 0x98;
        if (Rec != nullptr) E = Rec + 0x10;
    }
    if (!(HoldTime > 0.0f) || E == nullptr) return;    // time > 0

    float Dur = At<float>(Self, 0x30);
    float Ratio = Dur / HoldTime;
    float Progress = Ratio < 0.0f ? 0.0f : (Ratio < 1.0f ? Ratio : 1.0f); // clamp to [0, 1] (VA 0x1459a86f8 = 1.0)
    Unknown_1407ac590(E, Progress);                    // progress multicast at E+0x00
    if (HoldTime >= At<float>(Self, 0x30)) return;     // strict: completes only when Dur > HoldTime

    Unknown_140684de0(E + 0x28);                       // complete multicast at E+0x28

    // Popup close (only when the popup key resolves and the record is registered in the popup map).
    uint8 KeyTmp[8];
    *reinterpret_cast<void**>(KeyTmp) = At<void*>(E, 0x64);
    void* Popup = Unknown_1426e3bd0(KeyTmp);
    if (Popup != nullptr)
    {
        void* Ent = static_cast<uint8*>(Unknown_140811c70(Popup)) + 0x30;
        void* Map = At<void*>(Popup, 0x10);
        int32 I = At<int32>(Ent, 0x8);
        if (!(I > At<int32>(Map, 0x38)) && At<void**>(Map, 0x30)[I] == Ent)
            Unknown_1407f17d0(Popup);                  // RequestClose (asm verified; name from HANDOFF only)
    }

    // Commit (out = reused int32 at the find-out slot, value idx). Gate: flag E+0x20, object E+0x18, slot 0x28.
    void* CommitObj = At<int32>(E, 0x20) != 0 ? At<void*>(E, 0x18) : nullptr;
    if (CommitObj != nullptr && VSlot<bool (*)(void*)>(CommitObj, 0x28)(CommitObj))
        VSlot<void (*)(void*, void*)>(CommitObj, 0x50)(CommitObj, &Idx);

    Unknown_1407ae640(Obj);                            // panel clear (asm 0x140802416, receiver = panel object)
}

// Global held tick 140802440 (3e9). Receiver = interface at global handler+0x28.
void GlobalHeldTick_140802440(void* Self, float DeltaTime)
{
    uint8* Obj = static_cast<uint8*>(Self) - 0x28;     // handler object (asm: lea -0x28(%rdi))

    // Held-owner validity AND cached widget (weak at obj+0x48 == Self+0x20).
    bool bOk = false;
    if (Unknown_142ee7880(static_cast<uint8*>(Self) + 0x20))
    {
        uint8 SharedTmp[16] = {};
        void* Widget = Unknown_143d5a140(Unknown_142ee5dd0(static_cast<uint8*>(Self) + 0x20), SharedTmp);
        bOk = Widget != nullptr && *static_cast<void**>(Widget) != nullptr;
        // Inline controller release of SharedTmp (UNKNOWN name; asm 0x1408024a2-0x1408024d2).
    }
    if (!bOk)
    {
        GlobalClearHeldOwner_1407ae750(Obj);           // asm 0x1408027ff
        return;
    }

    // Row-null gate (row at Self+0x10: DataTable, RowName.Index +0x18, RowName.Number +0x1C).
    if (At<void*>(Self, 0x10) == nullptr && Unknown_142cd8ed0(At<int32>(Self, 0x18), 0)
        && At<int32>(Self, 0x1C) == 0) return;

    At<float>(Self, 0x8) += DeltaTime;                 // held duration (obj+0x30)

    // Key = {row 16B at Self+0x10, weak built from WeakGet(Self+0x20)}; asm temps at rbp-0x79 / rbp-0x69.
    uint8 KeyTmp[24] = {};
    std::memcpy(KeyTmp, static_cast<uint8*>(Self) + 0x10, 16);
    void* OwnerObj = Unknown_142ee5dd0(static_cast<uint8*>(Self) + 0x20);
    Unknown_142ee3b30(KeyTmp + 16, OwnerObj);          // writes the 8-byte weak into KeyTmp+16
    int32 Idx = Unknown_1407bd580(static_cast<uint8*>(Self) + 0x28, KeyTmp);
    void* RowRes = Unknown_14078c640(static_cast<uint8*>(Self) + 0x10, reinterpret_cast<void*>(kRowTableVA_1459fa130));
    if (Idx == -1 || RowRes == nullptr) return;

    if (!Unknown_142ee7880(static_cast<uint8*>(Self) + 0x38)) return;   // second weak at obj+0x60 (UNKNOWN role)
    uint8* Rec = static_cast<uint8*>(Unknown_1407c20f0(RowRes, Unknown_142ee5dd0(static_cast<uint8*>(Self) + 0x38)));
    if (Rec[0x19] == 0) return;                        // hold enabled (no null check in asm; D5)
    float HoldTime = *reinterpret_cast<float*>(Rec + 0x1C);
    if (!(HoldTime > 0.0f)) return;

    uint8* Base = At<uint8*>(Self, 0x28);
    uint8* E = Base + static_cast<int64>(Idx) * 0x90;  // record stride 0x90
    float Dur = At<float>(Self, 0x8);
    float Ratio = Dur / HoldTime;
    float Progress = Ratio < 0.0f ? 0.0f : (Ratio < 1.0f ? Ratio : 1.0f);
    Unknown_1407ac590(E + 0x20, Progress);             // progress multicast at E+0x20
    if (HoldTime >= Dur) return;                       // strict

    if (!Unknown_1407d0160(Obj, static_cast<uint8*>(Self) + 0x20)) return;   // external eligibility
    Unknown_140684de0(E + 0x48);                       // complete multicast at E+0x48

    if (At<int32>(E, 0x40) != 0)
    {
        void* Cm = At<void*>(E, 0x38);
        if (Cm != nullptr && VSlot<bool (*)(void*)>(Cm, 0x28)(Cm))
        {
            uint8 FalseByte = 0;
            VSlot<void (*)(void*, void*)>(Cm, 0x50)(Cm, &FalseByte);     // Commit(false)
        }
    }
    GlobalClearHeldOwner_1407ae750(Obj);
}

// Global clear 1407ae750. Receiver = global handler object.
void GlobalClearHeldOwner_1407ae750(void* GlobalHandlerObj)
{
    void* M = At<void*>(GlobalHandlerObj, 0x20);       // OuterPrivate = manager
    if (M == nullptr) return;

    void* Reg = Unknown_14080e600(GlobalHandlerObj);
    void* Map = At<void*>(M, 0x10);
    int32 I = At<int32>(Reg, 0x38);
    if (I > At<int32>(Map, 0x38)) return;              // signed compare, no negative check (asm)
    if (At<void**>(Map, 0x30)[I] != static_cast<uint8*>(Reg) + 0x30) return; // must be registered

    uint8 Tmp[8] = {};
    At<uint64>(GlobalHandlerObj, 0x48) = *static_cast<uint64*>(Unknown_142ee3b30(Tmp, nullptr)); // held owner weak cleared
    At<int32>(GlobalHandlerObj, 0x30) = 0;             // held duration cleared
    At<uint64>(GlobalHandlerObj, 0x38) = 0;            // held row (16 bytes) cleared
    At<uint64>(GlobalHandlerObj, 0x40) = 0;

    void* Cls = Unknown_140806b60();
    void* X = Unknown_142ec9900(GlobalHandlerObj, Cls);
    At<void*>(M, 0xB0) = X;                            // manager interface word
    At<void*>(M, 0xA8) = GlobalHandlerObj;             // manager object word
}

// Subsystem Deinitialize 1407b8610. Receiver = UCommonUISubsystem. Not SetLegacyInputEnabled(false).
void CommonUISubsystem_Deinitialize_1407b8610(void* Subsystem)
{
    void* M = At<void*>(Subsystem, 0x50);              // CommonInputManager (asm offset)
    if (M == nullptr) return;

    Unknown_140690620(static_cast<uint8*>(M) + 0x90, Subsystem); // NativeInputSuspensionChanged.RemoveAll(this)-shaped
    int32 Index = At<int32>(M, 0xC);                   // UObjectBase::InternalIndex
    // asm 0x1407b8651-0x1407b8669 (non-negative index): chunk = Index >> 16, sub = Index & 0xFFFF,
    // item = Chunks[chunk] + sub * 24 (FUObjectItem stride). Chunks at VA 0x148263a18; NumElements at 0x148263a2c.
    // asm checks only Index >= NumElements (jge); no index below zero check (D6).
    uint8* Chunk = *reinterpret_cast<uint8**>(kGUObjectArrayItems_148263a18 + static_cast<uintptr_t>(Index >> 16) * 8);
    uint8* Item = Chunk + static_cast<uintptr_t>(Index & 0xFFFF) * 24;
    At<uint32>(Item, 0x8) |= 0x20000000u;              // EInternalObjectFlags::PendingKill = 1 << 29 (ObjectMacros.h:541)
    // No clear of +0x50 and no broadcast (asm).
}
