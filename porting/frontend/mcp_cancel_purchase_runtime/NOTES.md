# FortMcpContext.CancelEpicPurchasingDisplay: blocked notes

## Evidence (verified)
- Function is referenced as a blueprint import in native-ui-startup logs (`/Script/FortniteGame.FortMcpContext:CancelEpicPurchasingDisplay`, WebPurchase), and listed in porting/HANDOFF.md as unresolved.
- Generated UFortMcpContext.h/.cpp has no declaration; the .cpp is 3 lines. Reflection JSON has no entry.
- RVA 0x1421cb230: loads this+0x378, tail-jumps to RVA 0x1411ce2d0 when nonzero.
- RVA 0x1411ce2d0: checks helper+0x50; builds FString "UserCancelled" (UTF-16 literal at 0x145c08630) and calls RVA 0x1411d0c30.
- RVA 0x1411d0c30: partial trace (see definitions.inc). Calls RVA 0x140d0e7f0 for an object stored into helper's Impl+0x50 shared slot.
- Engine FPurchaseReceipt exists at Engine/Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlinePurchaseInterface.h:95. FOnlineError and delegates FOnPurchaseCheckoutComplete (TSharedRef<FPurchaseReceipt>) are declared there too. Not proven that RVA 0x140d0e7f0 builds an FPurchaseReceipt.
- Caller seen: RVA 0x14515b410 (UFortRootViewportLayout clear path) calls RVA 0x1421cb230 on the result of RVA 0x1425dc880 (unnamed).

## Blocked
1. Type names and layouts for the helper at this+0x378, its Impl at helper+0x50, and Impl fields +0x10, +0x30, +0x38, +0x50.
2. Vtable slots 0x28 and 0x50 used on the Impl+0x30 object.
3. RVA 0x140d0e7f0: identity of the receipt/error object and its layout.
4. Full tail of RVA 0x1411d0c30 (after 0x1411d0dbf).
5. Whether the cancel path broadcasts FOnEpicPurchasingWidgetDismissed (DismissPurchaseWidget, generated header line 208). Not seen in the traced windows.
6. UFUNCTION flags and category: not in reflection JSON.
7. Identity of RVA 0x1425dc880.

## Files
- declarations.inc: declaration only (no parameters, void), usable once flags are confirmed.
- definitions.inc: verified control flow inside #if 0; no compilable body.
- Generated files, porting/native, gen_native_classes.py: untouched. Nothing built, run, or committed.
