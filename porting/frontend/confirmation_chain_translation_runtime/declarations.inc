// Declarations for the confirmation-window bot-click chain.
// Source: porting/generated/decomp-imports/Imports/*.c (Ghidra pseudo-C). Window offsets are from `this`.
// Not built, not run, not committed. UNKNOWN items are listed in NOTES.md.
// Reflected names (ConfirmationButtons, DeclineButton, ConfirmationDescription, EntryBox_SimpleButtons,
// CommonLoadGuard_LatentSpinner, BotClickButton) are used as-is. Everything else is address-suffixed.

// ---- UFortConfirmationWindow: class-body members (non-reflected) ----

// Reflected entry point. Exec 145218540 tail-jumps to body 144f54030 (definitions.inc).
UFUNCTION(BlueprintCallable)
void BotClickButton();

// Window +0x596, byte. Compared against 0 in NativeConfirmClicked144f5fbc0 (cmp %dl,0x596(%rdi)).
// No writer in the translated set. Owner is UNKNOWN (see NOTES.md).
uint8 NativeFlag596 = 0;

// Window +0x490, 8-byte slot. Passed by address (&W+0x490) as the handle argument of
// NativeDispatch144ce33d0 from NativeConfirmClicked144f5fbc0 only. Type UNKNOWN.
alignas(8) uint8 NativeMember490[8];

// Window +0x480, object pointer whose vtable slots 0x2c0 and 0x270 are called in NativeHelper144f75f90. Type UNKNOWN.
void* NativeMember480 = nullptr;

// Window +0x4f8, 8-byte value. Only tested for == 0 (in NativeHelper144f75f90). Type UNKNOWN.
uint64 NativeMember4F8 = 0;

// Window +0x5c8 (data ptr), +0x5d0 (ArrayNum). 16-byte elements, see FNativeEntry144f55ae0.
TArray<FNativeEntry144f55ae0> NativeEntryArray5C8;

// Window +0x5dc, int32 re-entrancy counter in NativeHelper144f55ae0 (increment before loop, decrement after).
int32 NativeCounter5DC = 0;

// Element of the array at window +0x5c8. Raw layout as read in pseudo-C:
//   +0x0 pointer (virtual slot 0x50 is called on it), +0x8 int32 Count. Remaining bytes UNKNOWN.
struct FNativeEntry144f55ae0
{
	void* Ptr;
	int32 Count;
	int32 NativeUnknownC;   // UNKNOWN: bytes +0xc..+0xf not read by the pseudo-C
};

// ---- Free helpers (non-reflected, bodies in definitions.inc or UNKNOWN) ----

// Gate on window W. Pins a weak object (see NOTES.md) and returns (flag & 0x19) != 0.
bool NativeWeakPin143d66ef0(UFortConfirmationWindow* W);

// Confirm-button helper. Button is the clicked UCommonButton.
void NativeConfirmClicked144f5fbc0(UFortConfirmationWindow* W, UCommonButton* Button);

// Decline helper. Second argument is passed by the caller (DeclineButton) but never read by the callee.
void NativeDeclineClicked144f5fce0(UFortConfirmationWindow* W, UCommonButton* UnusedDeclineButton);

// Present wrapper. bFlag is passed through as Dispatch's second argument. ValuePtr is dereferenced.
void NativePresent144f55c60(UFortConfirmationWindow* W, bool bFlag, const uint64* ValuePtr);

// Typed outer walk: returns the object found at W or W->Outer->Outer that matches the cached class, else null.
void* NativeOuterWalk144ade110(void* Start);

// Dispatch to the description's two objects. Controller is never read in the pseudo-C body.
// Desc points at a copy (or at W+0x4b0). Handle is undefined4* in pseudo-C; only *Handle is read.
void NativeDispatch144ce33d0(void* Controller, bool bFlag2, uint64 Value, uint8* Desc, bool bFlag5, uint32* Handle);

// Unnamed helpers from pseudo-C (u2 and u3 filenames).
void NativeHelper144f75f90(UFortConfirmationWindow* W);   // u2
void NativeHelper144f55ae0(UFortConfirmationWindow* W);   // u3

// Globals read by NativeOuterWalk144ade110 (addresses 0x1481134c8 and 0x148382580).
void*  NativeGlobal1481134c8 = nullptr;
uint64 NativeFName148382580 = 0;

// Shared class test used by 144f54030 and 144ade110. Body is in definitions.inc.
bool NativeClassIsChildOf144ade110(void* Obj, void* Cls);

// ---- UNKNOWN callees and vtable slots (declared only; bodies not in the translated set) ----
// Writes the pinned pair {object, shared block} for window W. Pair layout INFERRED from CALLEES.md, not in pseudo-C.
void   NativeWeakPinOut143d5a140(UFortConfirmationWindow* W, void* Out);
bool   NativeQuerySlot28_1b0(void* P);                                 // vtable 0x28 on pinned+0x1b0 (UNKNOWN)
void   NativeWriteSlot48_1b0(void* P, uint8* OutByte);                 // vtable 0x48 on pinned+0x1b0 (UNKNOWN)
void   NativeBlockSlot0(void* Block);                                  // vtable slot 0 on shared block (UNKNOWN)
void   NativeBlockSlot8Delete(void* Block, bool bDelete);              // vtable slot 8 on shared block (UNKNOWN)
void*  NativeDispatchSlot288(void* W);                 // vtable 0x288 on the window, returns object (UNKNOWN)
void*  NativeClassGetter142a3d9f0();                   // UNKNOWN UClass getter
void*  NativeClassGetter143dbe060();                   // UNKNOWN UClass getter
void   NativeDispatchSlot268(UCommonButton* Button);   // vtable 0x268 on Button, result unused (UNKNOWN)
void   NativeCopyDescription144f4cfd0(uint8* Out, const uint8* Src);   // copy-construct description (UNKNOWN)
void   NativeDestroyDescription144ae20d0(uint8* Desc);                 // destroy description temp (UNKNOWN)
void   NativeInit141f7ce20(uint32* Buf);                               // builds handle local (UNKNOWN)
void   NativeHandleOp141f8f0a0(uint32* Handle);                        // UNKNOWN
void   NativeHandleOp141f87900(uint32* Handle);                        // UNKNOWN
bool   NativeQuerySlot28_F8(void* Obj);                                // vtable 0x28 on desc+0xf8 object (UNKNOWN)
void   NativeDispatchSlot50_F8(void* Obj, bool bFlag, uint64 Value);   // vtable 0x50 (UNKNOWN)
bool   NativeQuerySlot28_108(void* Obj);                               // vtable 0x28 on desc+0x108 object (UNKNOWN)
void   NativeDispatchSlot50_108(void* Obj, bool bFlag, uint64 Value, bool bFlag5, uint32 HandleDword); // vtable 0x50 (UNKNOWN)
void*  NativeDispatchSlot290(void* Obj);                               // vtable 0x290 on matched object (UNKNOWN)
void*  NativeFunc144ade1e0(void* Global, void* Obj);                   // UNKNOWN
uint64 NativeInit142cbd480();                                          // UNKNOWN
void*  NativeInit142cc0600(uint64 A, uint64 FName);                    // UNKNOWN
void   NativeFunc143d852b0(void* EntryBox, bool bFlag);                // EntryBox setter (UNKNOWN meaning)
void   NativeFunc1407f7e20(void* LatentSpinner, bool bFlag);           // UNKNOWN meaning
void   NativeHelper142b1bea0(uint8* Buf);                              // UNKNOWN
void   NativeHelper144f4f6a0(uint8* Dst, uint8* Src);                  // UNKNOWN (arg direction as in pseudo-C)
void   NativeHelper144f555e0(UFortConfirmationWindow* W);              // UNKNOWN
void   NativeHelper144f7b690(UFortConfirmationWindow* W);              // UNKNOWN
void   NativeHelper144f7b730(UFortConfirmationWindow* W);              // UNKNOWN
void   NativeDispatchSlot2C0(void* Obj, const uint8* Desc);            // vtable 0x2c0 on W+0x480 (UNKNOWN)
void   NativeDispatchSlot270(void* Obj, bool bFlag);                   // vtable 0x270 on W+0x480 (tail jump in pseudo-C)
void   NativeHelper1407b7ed0(UFortConfirmationWindow* W);              // UNKNOWN
void   NativeHelper140685370(void* ArrayAddr, bool bFlag);             // UNKNOWN
bool   NativeDispatchSlot50_Entry(void* Ptr, UFortConfirmationWindow* W); // vtable 0x50 on u3 element (UNKNOWN)
