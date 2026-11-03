// Declarations for the legacy CommonUI input eligibility fragment (VA 0x1407d0160)
// and its CommonButton helper (VA 0x1407cf620).
// Image: image-125.runtime.bin; file offset = VA - 0x140000000.
// Names are project-local unless noted. Every callee not verified in detail is
// marked UNKNOWN (see NOTES.md, UNKNOWN list). No engine name is claimed where
// the body was not matched.

// Win64 pointer pair as returned through an out-pointer (object, shared control block).
// Layout: Object at +0, Ctrl at +8 (verified by asm at 1407d02d2/1407d02d6 and 143d5cb70).
struct LegacySharedPtrRaw
{
    void* Object = nullptr;
    void* Ctrl = nullptr;
};

// Opaque local path object constructed by 142f44d90 and destroyed by 14079aa60.
// Verified offsets only (relative to the object base, asm 1407d0373-1407d0391):
//   +0x008  inline element storage, 4 x 0x48 bytes
//   +0x128  heap data pointer (non-null means heap storage is used)
//   +0x130  int32 element count
struct LegacyLocalPath
{
    uint8 Raw[0x140];
};

// FString-shaped temporary as used by the log call sites (asm 1407d0060/0x50,0x58):
//   +0x0 data pointer, +0x8 int32 count.
struct LegacyFStringTemp
{
    const wchar_t* Data = nullptr;
    int32 Num = 0;
};

// Per-element record stride inside LegacyLocalPath (asm 1407d0373-1407d0394).
// Widget pointer at +0x38. Element type name UNKNOWN.
constexpr int32 LegacyPathElementStride = 0x48;
constexpr int32 LegacyPathElementWidgetOffset = 0x38;

// Log verbosity gate: DAT at image offset 0x80d1df0 (VA 0x1480d1df0), compared as unsigned byte
// against 6 (asm 1407d019c, 1407d0221, 1407d046c). Enum value 6 = Verbose (see NOTES.md).
extern uint8 LegacyGlobalInputLogVerbosity;      // image 0x80d1df0 (identity of owning category UNKNOWN)
extern uint8 LegacyGlobalInputLogCategory[];     // image 0x80d1df4 (FLogCategory object, layout UNKNOWN)
extern uint8 LegacyGlobalInputPathFlag;          // image 0x7c522f0 (passed as r9 to 14300d060; meaning UNKNOWN)
extern uint64 LegacyGlobalSlateObjectRef;        // image 0x8271318 (uVar7; meaning UNKNOWN)
extern const wchar_t LegacyEmptyWideString[];    // image 0x59a4b30, verified to be L""

// Format strings, image offsets verified by reading UTF-16 literals from the image:
//   0x59f2530  L"Global Input (Not Enabled, Ignoring): %s"
//   0x59f2590  L"Global Input (Interaction Not Enabled, Ignoring): %s"
//   0x59f2600  L"Global Input (%s !IsEnabled), Ignoring: %s"
//   0x59f2660  L"Global Input (%s !AreChildrenHitTestVisible), Ignoring: %s"
//   0x59f26e0  L"Global Input (No Path To Widget): %s"
extern const wchar_t LegacyFmtNotEnabled[];
extern const wchar_t LegacyFmtInteractionNotEnabled[];
extern const wchar_t LegacyFmtNotIsEnabled[];
extern const wchar_t LegacyFmtNotChildrenHitTestVisible[];
extern const wchar_t LegacyFmtNoPathToWidget[];

// Helper for 1407cf620 and its callees.
// 143d60da0 (UNKNOWN): returns a byte; first call in 1407cf620 (asm 1407cf62d).
uint8 NativeInputTypeQuery143d60da0(void* Widget);

// 143d5cb70: verified structure matches UWidget::GetIsEnabled
// (cached widget valid -> cached Slate enabled attribute; else bIsEnabled bit).
// Source: Runtime/UMG/Private/Components/Widget.cpp:242 (GetCachedWidget then IsValid ? IsEnabled : bIsEnabled).
// Slate attribute internals at +0x190/+0x198/+0x1a0 are UNKNOWN.
bool NativeGetIsEnabled143d5cb70(void* Widget);

// 143d5a140: sret accessor returning a shared pointer pair by pointer (rcx = this, rdx = out).
// Asm 143d5a14a-143d5a166 reads +0xe8 (ptr) and +0xf0 (control) with validity check on control count.
// Full body UNKNOWN beyond that. Not claimed as UWidget::GetCachedWidget.
LegacySharedPtrRaw* NativeGetCachedWidget143d5a140(void* Widget, LegacySharedPtrRaw* Out);

// Shared control block release (layout from asm, not from engine headers):
//   +0x8 shared count (int32), +0xc weak count (int32), vtbl[0] = dispose, vtbl[1] = delete(flag 1).
// Verified at 1407d02f9-1407d0318, 1407d0605-1407d0625, 143d5cbf0.
void NativeReleaseSharedControl(void* Ctrl);
void NativeAddRefSharedControl(void* Ctrl);     // 1407d032b, 1407d035a: incl 0x8(ctrl)

// 1407cf620 helper (CommonButton interaction check). Returns 1 only if
// GetIsEnabled, (flags@+0x2a1 & 6) == 6, and (byte)(type-1) >= 2 all hold.
bool NativeHelper1407cf620(void* Widget);

// 1407d0160 eligibility. rcx is overwritten at 1407d0185 before any read, so
// UnusedRcx is not used. RowInputRecord is rdx (caller 140802440 at 80262f passes
// rdx = this+0x20 and rcx = this-0x28; see NOTES.md).
bool NativeEligibility1407d0160(void* UnusedRcx, void* RowInputRecord);

// Record resolver (UNKNOWN). Called as FUN_142ee5dd0(param_2) at 1407d018b, 1407d01d5, 1407d02bd,
// 1407d0223, 1407d047c, 1407d04de, 1407d0589. Verified uses only: +0x10 owner, +0x18 name value.
void* NativeRecordLookup142ee5dd0(void* RowInputRecord);

// Context object (UNKNOWN). Asm 1407d01e6-1407d01fa: +0x30 array ptr, +0x38 int32 index.
void* NativeContext140807cd0();

// Local path constructor/destructor (UNKNOWN). 142f44d90 at 1407d02ae; 14079aa60 at 1407d062d.
void NativeLocalPathCtor142f44d90(LegacyLocalPath* Path);
void NativeLocalPathDtor14079aa60(LegacyLocalPath* Path);

// Path lookup (UNKNOWN). 14300d060 at 1407d0342 (rcx = global object, rdx = &pinned pair,
// r8 = path, r9b = global flag). Returns bool.
bool NativeFindPath14300d060(uint64 Global, LegacySharedPtrRaw* Pinned, LegacyLocalPath* Path, uint8 Flag);

// Owner-in-path check (UNKNOWN). 142f4c5b0 at 1407d0366 (rcx = path, rdx = &pinned pair). Returns bool.
bool NativeFindOwner142f4c5b0(LegacyLocalPath* Path, LegacySharedPtrRaw* Pinned);

// Widget name (UNKNOWN). 142fc3fe0 at 1407d04b7 and 1407d0519: (rcx = out buffer, rdx = widget).
LegacyFStringTemp* NativeWidgetName142fc3fe0(LegacyFStringTemp* Out, void* Widget);

// Name value to string (UNKNOWN). 142ce9f90 at 1407d01c4 etc.: (rcx = &name value, rdx = out string).
void NativeNameValueToString142ce9f90(const uint64* NameValue, LegacyFStringTemp* Out);

// Log emitter (UNKNOWN name). 142c54b70 at 1407d0277, 1407d054e, 1407d05da:
// called as (0, 0, &LegacyGlobalInputLogCategory, 6, Fmt, Arg1 [, Arg2]).
void NativeLog142c54b70(int32 Ctx0, int32 Ctx1, const void* Category, int32 Verbosity, const wchar_t* Fmt, const wchar_t* Arg1, const wchar_t* Arg2, int32 NumArgs);

// Free (142bee710 at 1407d0286, 1407d0553-1407d056c, 1407d05df-1407d05e9). Name UNKNOWN.
void NativeFree142bee710(void* Ptr);
