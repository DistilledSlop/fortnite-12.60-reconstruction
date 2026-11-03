// FortMcpContext::CancelEpicPurchasingDisplay chain (translation, not integrated).
// Source: decomp-imports CancelExec__1421cb230.c, CancelBody__1411ce2d0.c, CancelInner__1411d0c30.c.
// Disassembly check (image-125.runtime.bin): 0x1421cb230 loads this+0x378, returns when null,
// otherwise jumps to 0x1411ce2d0 (CancelBody). 0x1411ce2d0 tests helper+0x50 with cmpq $0.

// ===== Class body: UFortMcpContext =====
// Reflected name (blueprint import /Script/FortniteGame.FortMcpContext:CancelEpicPurchasingDisplay).
// UFUNCTION flags VERIFIED from fortnite_game_reflection.json (owner block_va 0x146433050 = UFortMcpContext):
// flags 0x4020403 = FUNC_Final|FUNC_RequiredAPI|FUNC_Native|FUNC_Public|FUNC_BlueprintCallable; params []. Category UNKNOWN.
void CancelEpicPurchasingDisplay();
// Helper pointer at this+0x378. Placement at offset 0x378 is UNKNOWN in the generated class layout.
NativeCancelHelper1421cb230* NativeCancelHelper378 = nullptr;

// ===== Namespace scope (after the class) =====
// Helper object. Layout UNKNOWN except +0x50 (Impl pointer) and +0x58 (controller pointer).
struct NativeCancelHelper1421cb230;
// Impl object at helper+0x50. Layout UNKNOWN beyond the offsets used in definitions.inc.
struct NativeCancelImpl1411d0c30;

// Native functions (address-suffixed).
void NativeCancelBody1411ce2d0(NativeCancelHelper1421cb230* Helper);
void NativeCancelInner1411d0c30(NativeCancelHelper1421cb230* Helper, const FString* Error);

// FString helpers (FString layout as used by the pseudo-C: Data +0x0, Num +0x8, Max +0xc).
// 0x140680160 is called with (FString*, 14). Semantics UNKNOWN beyond the call pattern.
void NativeFStringPrepare140680160(FString* Str, int32 Count);
// 0x1406800e0 is called when Max < Num. Semantics UNKNOWN (grow assumed from the call site only).
void NativeFStringResize1406800e0(FString* Str);
// 0x142bc7780 copies 0x1c bytes from a wide literal (14 TCHARs including terminator).
void NativeWideCopy142bc7780(void* Dst, const wchar_t* Src, int32 Bytes);
// 0x142bee710 frees a buffer when the pointer is non-null.
void NativeFreeData142bee710(void* Data);

// Logging. 0x142c54b70 takes (0, 0, category, verbosity, format, ...). Leading args UNKNOWN.
void NativeUELog142c54b70(int64 Arg0, int64 Arg1, const void* Category, int32 Verbosity, const wchar_t* Format, ...);
extern int32 NativeLogLevel1480eded8;            // DAT_1480eded8 (gates: > 4 success, > 2 failure)
extern const void* NativeLogCategory1480ededc;   // DAT_1480ededc
extern const wchar_t NativeEmptyStr1459a4b30[];  // DAT_1459a4b30 (used when Error.Num == 0)

// Shared-pointer and result helpers used by CancelInner (addresses are the call targets).
// 0x140d0e7f0 returns its out pointer (rax = r14) holding {Object, Controller}; the out slot is auStack_78.
// VERIFIED: Object = FPurchaseReceipt-sized 0x30 heap object (vptr, TransactionId@0x8, TransactionState@0x18,
// ReceiptOffers TArray@0x20); Controller = 0x18-byte TReferenceControllerWithDeleter-shaped block.
void** NativeCreateSharedResult140d0e7f0(void* Arg, void* OutSlot);
void NativeResultInit140d5ddf0(void* Result, bool bSucceeded);     // 0x140d5ddf0 (bool arg = uVar11)
void NativeResultSetError140d6ff70(void* Result, const FString* Error); // 0x140d6ff70

// Inline refcount sequence used at the CancelInner release sites. Shared count at ctrl+0x8,
// weak count at ctrl+0xc, vtable slot 0 destroys the object, vtable slot 1 (arg 1) deletes it.
// Null-safe. Not a binary function: the name is the project-local stand-in for the inline code.
void NativeCtrlRelease1411d0c30(void* Ctrl);

// Pseudo-C locals with no visible assignment (sources UNKNOWN). Left as undefined symbols on purpose.
extern void* UNKNOWN_plStack_70;   // released on the failure path
extern void* UNKNOWN_plStack_28;   // atomic release at the end of CancelInner
// lStack_40 / lStack_50 are VERIFIED as ResultBuf+0x18 / +0x8 and are now written in definitions.inc.
