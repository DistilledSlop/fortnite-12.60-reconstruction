// restart_modal_translation_runtime: declarations for the restart-modal click path.
// Fragment only. Not registered, not compiled. UNKNOWN items are listed in NOTES.md.
// Offsets verified with objdump on image-125.runtime.bin (VA - 0x140000000) where NOTES.md says so.
// Naming: reflected names as-is; non-reflected functions, members and globals carry their VA suffix.
// NativeCall<VA> = callee whose semantics are not established by the pseudo-C.

#include <cstdint>

// Raw vtable dispatch. SlotOffset is a byte offset into the vtable, as in the binary (*(this) + off).
template <typename... Args>
inline void NativeVtblCall(void* Obj, uint32_t SlotOffset, Args... A)
{
    void** Vtbl = *reinterpret_cast<void***>(Obj);
    auto Fn = reinterpret_cast<void (*)(void*, Args...)>(Vtbl[SlotOffset / sizeof(void*)]);
    Fn(Obj, A...);
}

// ---- Globals (addresses only; roles UNKNOWN unless noted) ----
extern int32_t NativeGlobalLogVerbosity1480d1cd8;  // DAT_1480d1cd8, compared as "> N" before each log
extern int32_t NativeLogCategory1480d1cdc;         // DAT_1480d1cdc, category passed by address (type placeholder)
extern int32_t NativeLogCategory1481348e4;         // DAT_1481348e4, launcher-branch category
extern int32_t NativeGlobalLogVerbosity1481348e0;  // DAT_1481348e0, launcher-branch verbosity
extern const wchar_t NativeEmptyString1459a4b30[]; // DAT_1459a4b30, "" fallback; content not verified
extern void* NativeGlobal1482675e8;                // _DAT_1482675e8, first arg of Deactivate log; role UNKNOWN
extern uint64_t NativeGlobal1480d2260;             // _DAT_1480d2260, read by NativeHelper140809130

// ---- Logging and helper calls (signatures UNKNOWN unless noted; args kept as pseudo-C shows them) ----
void NativeLog142c54b70(void* Unk0, void* Unk1, void* Category, int32_t Verbosity, const wchar_t* Fmt, ...);
void NativeLog142cc40c0(void* Category, const wchar_t* Fmt, const wchar_t* Arg);
// FName-to-string helper (role inferred from usage only). Returns FString* (Data @0, ArrayNum @8).
void* NativeCall142ce9f90(const uint64_t* In, void** OutBuf);
const wchar_t* NativeNameText(uint64_t NameIn);    // defined in definitions.inc
void NativeCall142bee710(void* Buf);               // buffer release
void NativeCall142ca88a0(const uint32_t In[4], FString* OutStr, int32_t Unk); // op describe, UNKNOWN role
uint64_t* NativeCall142cd49c0(uint64_t* OutTmp, const char* Text, int32_t Flag); // "LauncherCheck" literal
uint64_t NativeCall142cbd480();                    // thunk_FUN_142cbd480, UNKNOWN
void* NativeCall142cc0600(uint64_t A, uint64_t B); // thunk_FUN_142cc0600, UNKNOWN
bool NativeCall142bcb050();                        // platform restart check; nonzero = return
void NativeCall142ce8ac0(int32_t Arg);             // called with 0 in the restart path; role UNKNOWN
void NativeCall142c9ff10(uint32_t Payload[4]);     // per-op 16-byte payload, UNKNOWN meaning
uint64_t NativeCall142ec7730(void* Obj, uint64_t Arg); // UNKNOWN
bool NativeCall142ee7880(const void* WeakPtr);     // IsValid on weak at +0x3b0 (BEHAVIOR.md)
void* NativeCall142ee5dd0(const void* WeakPtr);    // Get on weak at +0x3b0 (BEHAVIOR.md)
void NativeCall142ee3b30(void* WeakPtr);           // weak op on +0x3b0, UNKNOWN
bool NativeCall142cd8ed0(int32_t A, int32_t B);    // UNKNOWN
void NativeCall1407bdba0(void* Array, int32_t* OutIdx, const void* Key); // index into array, result in OutIdx[0]
void NativeCall1407ac590(void* Elem, int32_t Zero);                      // UNKNOWN
void NativeCall140684de0(void* Obj);                                     // UNKNOWN
void NativeCall140680160(void* Buf, int32_t N);                          // UNKNOWN
void NativeCall1406800e0(void* Buf, uint64_t N);                         // UNKNOWN
void NativeCall140789600(void* Dst, int32_t N, const char* Src, int32_t N2, int32_t Mask); // UNKNOWN
void NativeCall140786850(void* Delegate, uint32_t FnLow, void* Record, const uint32_t* Words); // bind, UNKNOWN
void NativeCall1408090b0(void* Obj);                                     // UNKNOWN
void NativeCall1408090f0();                                              // UNKNOWN
void NativeCall1407fc000();                                              // UNKNOWN
void NativeFn1407dc080();                                                // address used as a value
void NativeFn1407dbc80();                                                // address used as a value
void NativeCall1407b7ed0(void* Self);                                    // fallback; body not in pseudo-C
void NativeGrow14069ea30(void* Array, int32_t NewIndex);                 // TArray grow (+0xf4 path)
void NativeCall1407a95e0(void* Manager, void* Op);                       // op helper; pseudo-C ignores its return
void NativeCall1407ad3f0(void* Manager, void* OutStackArray);            // predicted stack copy
void NativeCall1407ab920(void* Manager);                                 // begin op processing

// ---- Reflected names used by the restart path ----
// UFortRestartClientModal (UFortActivatablePanel : UCommonActivatablePanel)
//   UFUNCTION(BlueprintCallable) void HandleButtonClicked();   // exec 1452c2a80 per HANDOFF.md
//   bool bNativeRestartFlag448 = false;                         // +0x448, movzbl 0x448(%rcx) verified
// UCommonInputManager
//   TArray<UCommonActivatablePanel*> ActivatablePanelStack;     // data +0xb8, count +0xc0 (verified)
//   TArray<FNativeOperationRaw> Operations;                     // data +0xe8, count +0xf0, cap +0xf4 (verified)
//   bool bNativeFlagE0 = false;                                 // +0xe0 (verified read)
// UCommonActivatableWidget
//   bool bIsActive;                                             // byte +0x288 (offset from HANDOFF/pseudo-C)

// ---- Non-reflected members on UCommonActivatablePanel (types as the pseudo-C uses them) ----
//   TWeakObjectPtr<UCommonInputManager> NativeInputManagerWeak3B0;  // +0x3b0
//   bool bNativeStackMembership304 = false;                         // +0x304 (verified cmpb in RestartClick)
//   bool bNativeFlag303 = false;                                    // +0x303 (verified cmpb in PanelOutroReq)
//   float NativeFloat308;                                           // +0x308, compared with 0.0f
//   void* NativePtr3C0;                                             // +0x3c0 (8 bytes)
//   int32_t NativeInt3C4, NativeInt3C8, NativeInt3CC;               // +0x3c4, +0x3c8, +0x3cc (roles UNKNOWN)
//   TArray<uint8_t> NativeArray360;                                 // +0x360, element stride 0x98, struct UNKNOWN
//   void* NativeCallback3D0;                                        // +0x3d0 bind target (type UNKNOWN)
//   void* NativeCallback3E0;                                        // +0x3e0 bind target (type UNKNOWN)
//   uint64_t NativeField18;                                         // +0x18 identity UNKNOWN (NOTES.md)

// Operation element, 0x28 bytes. Layout taken from the pseudo-C stores (no reflected struct used).
// Operation values verified in this path: Intro = 0, Pop = 3.
struct FNativeOperationRaw
{
    uint8_t Operation;                 // +0x00 (EOperation; other values UNKNOWN here)
    void* Panel;                       // +0x08
    uint8_t NativeOpFlag10;            // +0x10 (set to 1 in both writers)
    uint8_t NativeOpFlag11;            // +0x11 (set to 1 in both writers)
    uint8_t NativeOpFlag12;            // +0x12 (set to 1 in both writers)
    uint8_t NativeOpFlag13;            // +0x13 (never set by the pseudo-C; UNKNOWN)
    uint32_t NativeOpPayload14[4];     // +0x14..+0x23, 16 bytes from NativeCall142c9ff10
    uint32_t NativeOpPad24;            // +0x24 (never set by the pseudo-C; UNKNOWN)
};

// ---- Function declarations (class-body style) ----
// UFortRestartClientModal
//   UFUNCTION(BlueprintCallable) void HandleButtonClicked();
// UCommonActivatablePanel (non-reflected)
//   void NativePanelOutroReq1407ce810();                               // vtable +0x4a8
//   void NativeOutroDefault1407bb630();                                // vtable +0x4b0 (entry RDX forwarded, UNKNOWN type)
//   void NativePanelPop1407e1130();                                    // vtable +0x4c0
//   void NativeHelper1407ae640();                                      // Helper_ae640
//   void NativeHelper140809130();                                      // Helper_809130 (dispatches vtable +0x210)
// UCommonInputManager (non-reflected)
//   void NativePopActivatable1407e0c80(UCommonActivatablePanel* ActivatablePanel);
// Free function
//   void NativeRestartPlatform141ed81a0(char Mode, char bExitFlag);
//   void NativePushWriter1407fc713(void* ObjRbx, void* ObjR14, UCommonActivatablePanel* PanelRbpM31,
//                                  uint8_t RdiOpArg, uint8_t OpByte28, uint8_t OpByte29, const uint32_t PayloadRbpM25[4]);
