# FortMcpContext::CancelEpicPurchasingDisplay: translation notes

Sources (decomp-imports/Imports): CancelExec__1421cb230.c, CancelBody__1411ce2d0.c, CancelInner__1411d0c30.c.
Disassembly cross-check: image-125.runtime.bin (file offset = VA - 0x140000000).
- 0x1421cb230: `cmpq` on this+0x378, `ret` when null, `jne 0x1411ce2d0` otherwise (CancelBody tail jump).
- 0x1411ce2d0: `cmpq $0, 0x50(%rcx)`; `call 0x680160` with edx=14; Num += 14; compare with Max; call 0x6800e0 when needed.
- 0x1411d0c30: `lea 0x50(%rcx),%rsi` (plVar1 = &helper+0x50), `cmpq $0, 0x50(%rcx)` on the Impl (success check).

## Naming applied
- Reflected: CancelEpicPurchasingDisplay (from blueprint import, not from reflection JSON).
- Native (address-suffixed): NativeCancelBody1411ce2d0, NativeCancelInner1411d0c30, NativeFStringPrepare140680160,
  NativeFStringResize1406800e0, NativeWideCopy142bc7780, NativeFreeData142bee710, NativeUELog142c54b70,
  NativeCreateSharedResult140d0e7f0, NativeResultInit140d5ddf0, NativeResultSetError140d6ff70,
  NativeCtrlRelease1411d0c30 (stand-in for the inline refcount sequence), NativeCancelHelper1421cb230,
  NativeCancelImpl1411d0c30, NativeCancelHelper378 (member), NativeLogLevel1480eded8, NativeLogCategory1480ededc,
  NativeEmptyStr1459a4b30.
- FString layout (Data +0, Num +0x8, Max +0xc) is confirmed by the pseudo-C pattern and by the disassembly (0x2c = Max).

## UNKNOWN
1. Class layout: whether UFortMcpContext has a member at +0x378 (helper pointer) is not confirmed.
2. UFUNCTION flags and category for CancelEpicPurchasingDisplay (no reflection entry).
3. Helper type and Impl layout beyond +0x50 (Impl), +0x10, +0x30, +0x38, +0x40, +0x48, +0x58.
4. Impl+0x10 contents: the argument passed to 0x140d0e7f0 (type unknown).
5. Return and object types of 0x140d0e7f0 (pair {Object, Controller} shape only).
6. Semantics of 0x140680160 (Num/Max effect) and 0x1406800e0 (resize). Only the call pattern is known.
7. Leading arguments (0, 0) of the log call 0x142c54b70; verbosity enum values and category type.
8. Virtual slots: Impl+0x30 object slot 0x28 (bool) and 0x50 (void); Impl+0x40 listener slot 0x48 (args: result, pair).
9. Source of plStack_70, plStack_28, lStack_40, lStack_50 (no visible assignment in the pseudo-C).
10. Whether the Result buffer (auStack_58) size is 0x40 or larger, and whether lStack_40/50 are its fields.
11. The pseudo-C compares &(Impl+0x50) with &uStack_88 (a stack local). It is kept literally, not interpreted.
12. Whether the cancel path broadcasts FOnEpicPurchasingWidgetDismissed (DismissPurchaseWidget, generated line 208).
    Not traced here.

## Observations (not fixed)
- The listener at Impl+0x40 is null when Impl+0x48 == 0; the pseudo-C then dispatches slot 0x48 on it unguarded.
- Helper+0x50 is cleared without a release of the Impl object (pseudo-C shows a plain store).
