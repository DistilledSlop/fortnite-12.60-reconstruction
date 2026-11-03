# FortMcpContext::CancelEpicPurchasingDisplay: translation notes

Sources (decomp-imports/Imports): CancelExec__1421cb230.c, CancelBody__1411ce2d0.c, CancelInner__1411d0c30.c.
Disassembly cross-check: image-125.runtime.bin (file offset = VA - 0x140000000).
- 0x1421cb230: `cmpq` on this+0x378, `ret` when null, `jne 0x1411ce2d0` otherwise (CancelBody tail jump).
- 0x1411ce2d0: `cmpq $0, 0x50(%rcx)`; `call 0x680160` with edx=14; Num += 14; compare with Max; call 0x6800e0 when needed.
- 0x1411d0c30: `lea 0x50(%rcx),%rsi` (plVar1 = &helper+0x50), `cmpq $0, 0x50(%rcx)` on the Impl (success check).

## Naming applied
- Reflected: CancelEpicPurchasingDisplay (blueprint import; also present in fortnite_game_reflection.json, see V1).
- Native (address-suffixed): NativeCancelBody1411ce2d0, NativeCancelInner1411d0c30, NativeFStringPrepare140680160,
  NativeFStringResize1406800e0, NativeWideCopy142bc7780, NativeFreeData142bee710, NativeUELog142c54b70,
  NativeCreateSharedResult140d0e7f0, NativeResultInit140d5ddf0, NativeResultSetError140d6ff70,
  NativeCtrlRelease1411d0c30 (stand-in for the inline refcount sequence), NativeCancelHelper1421cb230,
  NativeCancelImpl1411d0c30, NativeCancelHelper378 (member), NativeLogLevel1480eded8, NativeLogCategory1480ededc,
  NativeEmptyStr1459a4b30.
- FString layout (Data +0, Num +0x8, Max +0xc) is confirmed by the pseudo-C pattern and by the disassembly (0x2c = Max).

## VERIFIED (this pass; evidence in brackets)
- V1 UFUNCTION flags 0x4020403 = FUNC_Final|FUNC_RequiredAPI|FUNC_Native|FUNC_Public|FUNC_BlueprintCallable
  [reflection JSON, entry CancelEpicPurchasingDisplay flags ["0x4020403"], params []; owner block_va 0x146433050 =
  UFortMcpContext per native_classes_index.json; flag bits from CoreUObject UObject/Script.h]. Category not present.
- V2 CancelExec loads this+0x378 [0x1421cb230: `mov 0x378(%rcx),%rcx; test; jne 0x1411ce2d0; ret`].
  No reflected property sits at 0x378: neighbours are 0x318 EUCountryCodes and 0x3f8 DisplayPurchaseWidget [reflection JSON].
- V3 Literal L"UserCancelled\0" (0x1c bytes) is at VA 0x145c08630, the `lea` target in CancelBody
  [0x1411ce2d0 window: `lea ...(%rip) # 0x5c08630; mov $0x1c,%r8d`; bytes verified in image].
- V4 0x140d0e7f0 returns its out pointer (`mov %r14,%rax`) holding {Object@0, Controller@8}; CancelInner reads puVar8[0]/[1].
- V5 Object: `mov $0x30` allocation, vptr store, TransactionState byte at +0x18 (`mov %al,0x18(%rcx)` from src+0x60),
  FString copy to +0x8, TArray at +0x20 (Data 0x20, Num 0x28, Max 0x2c, element stride `shl $6` = 0x40).
  Matches FPurchaseReceipt [OnlinePurchaseInterface.h:95-96 base FOnlineJsonSerializable; 182-187 members;
  JsonSerializerMacros.h:16 `virtual ... override` implies a vptr] => sizeof 0x30 with vptr.
  FReceiptOfferEntry = 0x40 with vptr: 8 + FString 16 + FString 16 + int32 + pad + TArray 16 (header lines 128-146;
  FOfferNamespace/FUniqueOfferId = FString, OnlineStoreInterfaceV2.h:22-23).
  State gate `(state-2)&~4 == 0` = {2,6} = {Purchased, Restored} [enum order, OnlinePurchaseInterface.h:70-82].
  Handoff claim "0x140d0e7f0 builds FPurchaseReceipt": CONFIRMED by layout only (no RTTI name in the binary).
- V6 Controller: 0x18 bytes, vtable@0, Shared=1@0x8, Weak=1@0xc, Object@0x10 [0x140d0e7f0 stores; Templates/
  SharedPointerInternals.h:71-125 FReferenceControllerBase (vtable, SharedReferenceCount, WeakReferenceCount),
  TReferenceControllerWithDeleter (Object after base), ctor counts = 1 at lines 76-77]. Addref at +0x8 matches.
- V7 Impl+0x50 (Object) / Impl+0x58 (Controller) form a TSharedPtr-shaped pair: failure path stores the 0x140d0e7f0
  result there, CancelInner copies them into the call pair [pseudo-C CancelInner lines 81-89, 120-121;
  SharedPointerInternals.h:196-199 Object then ReferenceController].
- V8 Impl+0x40 / Impl+0x48 = FDelegateBase storage: allocator pointer @0 (default NUM_DELEGATE_INLINE_BYTES 0 =>
  FHeapAllocator) and int32 DelegateSize @8; `GetDelegateInstanceProtected` = `DelegateSize ? alloc : nullptr`
  [Delegates/DelegateBase.h:41-256, 229-232; Misc/CoreDefines.h:42-43]. Native (non-dynamic) delegate storage: confirmed.
  Signature arity: dispatch passes (instance, &FOnlineError, &TSharedRef<FPurchaseReceipt>) = Execute of
  DECLARE_DELEGATE_TwoParams(FOnPurchaseCheckoutComplete / FOnPurchaseRedeemCodeComplete) [OnlinePurchaseInterface.h:209,214].
  Which of the two delegates is bound: not verified.
- V9 ResultBuf is FOnlineError: 0x140d5ddf0 writes bSucceeded @0 (`mov %dl,(%rcx)`), zeroes +0x8..+0x27, FText init
  at +0x28, Result @0x40 = 0x12 (Unknown). 0x140d6ff70 copies the Error FString into +0x8 and +0x18 and sets
  Result @0x40 = 0x11 (FailExtended) [OnlineError.h:159-189; EOnlineErrorResult indices, no explicit values;
  Text.h:724-727 FText = TSharedRef + uint32 => 0x18]. sizeof(FOnlineError) = 0x48. Pseudo-C 0x40 REFUTED.
- V10 lStack_50 = auStack_58+0x8 (ErrorRaw data) and lStack_40 = auStack_58+0x18 (ErrorCode data): stack-offset
  arithmetic from the pseudo-C locals plus the writes in V9.
- V11 0x140d0e7f0 is called with Impl+0x10 as its first argument (rcx) and reads src+0x50 (FString), src+0x60 (state
  byte). The src type itself is not identified.

## UNKNOWN (remaining)
1. Type of the member at this+0x378 (NativeCancelHelper). Placement VERIFIED (V2); type/size not.
2. UFUNCTION category for CancelEpicPurchasingDisplay (flags verified, V1).
3. Helper and Impl layouts beyond the verified offsets (Helper+0x50/+0x58 pattern only; Impl+0x30, +0x38 unidentified).
4. Impl+0x10 type (argument to 0x140d0e7f0, V11).
5. Object identity beyond layout: the 0x30-byte object is FPurchaseReceipt by layout only (V5).
6. Semantics of 0x140680160 (Num/Max effect) and 0x1406800e0 (resize). Only the call pattern is known.
7. Leading arguments (0, 0) of log call 0x142c54b70; verbosity values 5/3 and category type; gates 4/2 are as in pseudo-C.
8. Virtual slots: Impl+0x30 slot 0x28 (bool) and 0x50 (void) on an unidentified object; listener slot 0x48 (args: instance,
   result, pair; arity matches Execute of a two-param native delegate, V8). The index 9 depends on the IDelegateInstance
   vtable layout (TryGetBoundFunctionName macro, dtor slot count, ABI) which is not fixed by the sources here.
9. Source of plStack_70 and plStack_28 (no assignment in the pseudo-C). plStack_70 may equal the controller held in
   the local that is released at LAB_1411d0d3b; identity not verified.
10. (Resolved in V9/V10; no longer UNKNOWN.)
11. The pseudo-C compares &(Impl+0x50) with &uStack_88 (a stack local). Kept literally, not interpreted.
12. Whether the cancel path broadcasts FOnEpicPurchasingWidgetDismissed. Not traced. The only delegate dispatch in the
    chain is the 2-argument listener call (CancelInner line 142); FOnEpicPurchasingWidgetDismissed is declared with
    DECLARE_DYNAMIC_DELEGATE and no parameters (native/Source/FortniteGame/Generated/FOnEpicPurchasingWidgetDismissed.h:10).
    Identity of the listener is not established.

## Observations (not fixed)
- The listener at Impl+0x40 is null when Impl+0x48 == 0; the pseudo-C then dispatches slot 0x48 on it unguarded.
- Helper+0x50 is cleared without a release of the Impl object (pseudo-C shows a plain store).
