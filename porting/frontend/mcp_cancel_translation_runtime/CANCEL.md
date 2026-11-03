# FortMcpContext::CancelEpicPurchasingDisplay: triage of UNKNOWNs

Evidence: NOTES.md V1-V11, definitions.inc, decomp-imports Imports/CancelExec__1421cb230.c, CancelBody__1411ce2d0.c,
CancelInner__1411d0c30.c, and new decompiles FortMcpClassNoRegister__142990750.c, FortMcpClassCtor__1429917e0.c,
FortMcpRegisterNatives__1429929b0.c, FortMcpCancelExecThunk__142993d30.c (same folder tree). Engine source under
/opt/fortnite/UnrealEngine/Engine/Source (and Engine/Plugins/Online/OnlineSubsystem/Source/Public).

## Step 1: classification of every UNKNOWN

Class A = affects observable behavior. Class B = Windows layout artifact, no behavior change in C++.

| # | UNKNOWN | Class | Reason |
|---|---------|-------|--------|
| 1 | Type/size of this+0x378 (Helper) | A | Null gate; decides whether the cancel does anything. |
| 2 | UFUNCTION category | B | Editor/Blueprint metadata only; no effect on the body. Flags verified (V1). |
| 3 | Helper layout beyond +0x50/+0x58 | B | Offsets not read on the traced path. |
| 3b | Impl+0x30 / +0x38 | A | Gate a virtual call (slot 0x28 then 0x50). |
| 4 | Impl+0x10 type (arg to 0x140d0e7f0) | A | Source of the receipt built on the failure path. |
| 5 | Object identity beyond layout (FPurchaseReceipt) | A | Payload broadcast and success gate. |
| 6 | 0x140680160 / 0x1406800e0 semantics | B | String allocation only. C++ equivalent is FString(TEXT("UserCancelled")) (see Step 2, item T). |
| 7 | Log call leading args (0,0), category type | A | Log output is observable. Arguments not verified. |
| 8 | Virtual slots (0x28, 0x50 on Impl+0x30; 0x48 on listener) | A | The slot numbers are B-type ABI artifacts, but the callee identity is unknown. |
| 9 | Sources of plStack_70 and plStack_28 | A | Released objects; which object is destroyed is observable. |
| 11 | Compare of Impl+0x50 with &uStack_88 | A | Selects the release branch; kept literal. |
| 12 | Dismissed broadcast on the cancel path | Resolved, see Step 2 item D. The listener body is still A. |
| O | Unguarded listener call when Impl+0x48 == 0 | A | Observed: a vtable call on null (crash). |

## Step 2: verified facts (new in this pass)

- D (Dismissed). UFortMcpContext reflected property DismissPurchaseWidget is at offset 0x408 (1032), kind Delegate,
  type FOnEpicPurchasingWidgetDismissed (DECLARE_DYNAMIC_DELEGATE, no parameters) [fortnite_game_reflection.json;
  native/Source/FortniteGame/Generated/UFortMcpContext.h:208, FOnEpicPurchasingWidgetDismissed.h:10].
  The cancel chain never reads +0x408: no 0x408 or 1032 reference in CancelExec, CancelBody, CancelInner, or the
  exec thunk 0x142993d30. The only dispatch in the chain is the native listener at Impl+0x40 (V8). So the cancel
  path does not broadcast DismissPurchaseWidget directly. Whether the listener itself triggers it is unknown.
- P (exec entry). Native table DAT_146438110 (99 entries, registered by FUN_1429929b0 -> FUN_142d6f540) maps
  "CancelEpicPurchasingDisplay" (entry 3) to 0x142993d30. That thunk runs `Stack.Code += !!Stack.Code`
  (P_FINISH, Runtime/CoreUObject/Public/UObject/ScriptMacros.h:94) and then contains the same body as CancelExec,
  inline. So CancelExec 0x1421cb230 is the same body without the FFrame step.
- C (class). Class registration FUN_142990750 calls GetPrivateStaticClassBody with InSize 0x468 and
  ClassConstructor = 0x1429917e0 (argument order checked in Runtime/CoreUObject/Private/UObject/Class.cpp:5361).
- Z (constructor). 0x1429917e0 zeroes param_1[0x6f] = object+0x378 (0x6f*8 = 0x378). So +0x378 is a native,
  null-initialised member. Its type is still unknown.
- S (no non-null store found). The class's own constructor does not store a non-null value to +0x378. A raw scan of
  the image for disp32 0x378 in mov/movq/movl/add/cmp forms found 1035 hits. Only 0x142991b4d, 0x142998ed5 and
  0x14299ebd6 fall in the FortMcpContext address band. Those are struct initialisation sequences that were not
  attributed to FortMcpContext. The scan misses SSE and lea-based copies. The writer of a non-null helper is not
  identified.
- T (string). Literal "UserCancelled" is 13 characters, so Num = 14 including the terminator, matching the
  0xe count in the pseudo-C. On Linux (Core/Public/Unix/UnixPlatform.h:53) PLATFORM_TCHAR_IS_4_BYTES = 1, so
  TCHAR is 4 bytes. The Windows copy size 0x1c (= 14 x 2-byte TCHAR) must not be hard-coded in the rebuilt project.
  Use TEXT("UserCancelled") instead.
- G (log gates). ELogVerbosity: NoLogging=0, Fatal=1, Error=2, Warning=3, Display=4, Log=5, Verbose=6,
  VeryVerbose=7 (Runtime/Core/Public/Logging/LogVerbosity.h). Gate `> 4` guards the Log(5) "Purchase Flow succeeded",
  and gate `> 2` guards the Warning(3) "Purchase Flow failed with error: %s". Category object still unknown.
- R (result). EOnlineErrorResult: Success=0, ..., FailExtended=17 (0x11), Unknown=18 (0x12), no explicit values
  (OnlineError.h). Confirms V9. FOnlineError members: bSucceeded, ErrorRaw, ErrorCode (FString), ErrorMessage (FText),
  Result (OnlineError.h:159-190).
- L (listener shape). FOnPurchaseCheckoutComplete and FOnPurchaseRedeemCodeComplete are both
  DECLARE_DELEGATE_TwoParams(const FOnlineError&, const TSharedRef<FPurchaseReceipt>&)
  (Engine/Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlinePurchaseInterface.h:209, 214). Which one is
  bound is not verified.
- Ordering. CancelInner calls the listener before clearing Helper+0x50 (`*HelperImplSlot = nullptr`). The listener
  can observe Helper->Impl still set. CancelInner also calls NativeResultSetError with the same "UserCancelled"
  string on both the success and failure branches, so the success result still carries ErrorRaw/ErrorCode
  "UserCancelled" with bSucceeded = true.

## C++ body (verified pieces only; UNKNOWN marked)

```cpp
// UFUNCTION flags 0x4020403, no parameters. Category UNKNOWN (B, no effect).
void UFortMcpContext::CancelEpicPurchasingDisplay()
{
    NativeCancelHelper* Helper = NativeCancelHelper378;   // this+0x378. Type UNKNOWN (A). Null-initialised (Z).
    if (Helper == nullptr) return;                        // 0x1421cb230 / thunk 0x142993d30
    if (Helper->Impl == nullptr) return;                  // Helper+0x50 (CancelBody cmpq $0)

    FString Reason = TEXT("UserCancelled");               // T: equivalent of 0x140680160 + 0x142bc7780 (B)
    NativeCancelInner(Helper, Reason);
}

void NativeCancelInner(NativeCancelHelper* Helper, const FString& Error)
{
    NativeCancelImpl* Impl = Helper->Impl;                // Helper+0x50
    bool bSucceeded = false;

    if (Impl->Receipt != nullptr)                          // Impl+0x50 (Object) non-null = success
    {
        // G: if (NativeLogLevel > 4) UE_LOG(<category UNKNOWN>, Log, TEXT("Purchase Flow succeeded"));
        bSucceeded = true;
    }
    else
    {
        // G: if (NativeLogLevel > 2) UE_LOG(<category UNKNOWN>, Warning,
        //        TEXT("Purchase Flow failed with error: %s"), Error.Num() == 0 ? <empty> : *Error);
        // A: NativeCreateSharedResult140d0e7f0(Impl+0x10 [UNKNOWN type]) -> {Object, Controller}.
        //    Stored into Impl+0x50/+0x58 with the release sequence (old controller released). Then
        //    release UNKNOWN_plStack_70 (source UNKNOWN).
        Impl->ReceiptObject = /* UNKNOWN: receipt from 0x140d0e7f0 */;   // TSharedPtr<FPurchaseReceipt>-shaped (V7)
    }

    // A: if (Impl+0x38 != 0 && Impl+0x30 != nullptr && Impl+0x30->Virt28()) Target->Virt50();  (both UNKNOWN)

    FOnlineError Result;                                   // 0x48 bytes (V9)
    Result.bSucceeded = bSucceeded;                        // NativeResultInit140d5ddf0
    Result.ErrorRaw   = Error;                             // NativeResultSetError140d6ff70 (V9)
    Result.ErrorCode  = Error;
    Result.SetResult(EOnlineErrorResult::FailExtended);   // 0x11 (R)
    // ErrorMessage: default (FText at +0x28, initialised by 0x140d5ddf0)

    TSharedRef<FPurchaseReceipt>-shaped Pair = { Impl->ReceiptObject, Impl->ReceiptController };  // addref controller

    // L: listener = (Impl+0x48 == 0) ? nullptr : Impl+0x40. Type UNKNOWN (FOnPurchaseCheckoutComplete or
    //    FOnPurchaseRedeemCodeComplete). Unguarded call; crashes if null (O).
    Listener->Execute(Result, Pair);                       // slot 0x48 is B-index; target identity UNKNOWN (A)

    release(Pair.Controller);
    Helper->Impl = nullptr;                                // *HelperImplSlot = 0 (verified)
    if (Helper->Controller != nullptr)                     // Helper+0x58 released (verified)
    {
        release(Helper->Controller);
        Helper->Controller = nullptr;
    }
    release(UNKNOWN_plStack_28);                           // source UNKNOWN
}
```

## Still lacks a verified implementation (remains UNKNOWN)

1. Type, size and writer of this+0x378 (NativeCancelHelper). Only placement, null init and the disp32 scan are verified.
2. Helper and Impl layouts beyond +0x50/+0x58 and Impl+0x10, +0x30, +0x38, +0x40, +0x48.
3. What 0x140d0e7f0 builds from Impl+0x10 (src+0x50 FString, src+0x60 state byte). Object is FPurchaseReceipt by layout only.
4. Semantics of 0x140680160 and 0x1406800e0 (only the call pattern is verified; B).
5. Log category object DAT_1480ededc and the leading (0,0) arguments of 0x142c54b70.
6. Which delegate (FOnPurchaseCheckoutComplete vs FOnPurchaseRedeemCodeComplete) is bound at Impl+0x40, and what it does.
7. Virtual-call targets: Impl+0x30 slots 0x28 and 0x50, and the listener slot 0x48. Index-to-function mapping needs the
   IDelegateInstance / object vtable layout.
8. Sources of UNKNOWN_plStack_70 and UNKNOWN_plStack_28.
9. Control-flow compare `Impl+0x50 == &uStack_88` (stack-address compare, kept literal).
10. Whether the listener triggers DismissPurchaseWidget (0x408). Not on the chain itself.
11. A complete writer search for +0x378 (SSE and lea-based stores not covered by the disp32 scan).
