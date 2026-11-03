# UFortRestartClientModal::HandleButtonClicked: C++ form and UNKNOWN triage

Evidence: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (file offset = VA - 0x140000000), disassembled with `objdump -D -b binary -mi386:x86-64` on small windows. Engine: `/opt/fortnite/UnrealEngine/Engine/Source`. Project headers: `porting/native/Source/*/Generated`. Frozen docs: `porting/frontend/panel_pop_stack_runtime/BEHAVIOR.md`, `WRITERS.md`, `porting/frontend/common_activatable_panel_native_events.md`, `porting/frontend/confirmation_click_runtime/RESTART_CALL.md`. Nothing built or run.

## 1. Verified control flow (this pass)

| Step | Address | Verified bytes / behavior |
|---|---|---|
| Entry | 0x144f5f9f0 | `movzbl 0x448(%rcx),%edx` (exit flag = byte +0x448), `mov $1,%cl`, `call 0x141ed81a0` at 0x144f5fa02 |
| Gate | 0x144f5fa07 | `cmpb $0,0x304(%rbx)`; `je 0x144f5fa22` |
| Pop | 0x144f5fa13 | `mov (%rbx),%rax`; `jmp *0x4c0(%rax)` (vtable slot +0x4c0) |
| Fallback | 0x144f5fa27 | `jmp 0x1407b7ed0` |
| Fallback body | 0x1407b7ed0 | `cmpb $0,0x288(%rcx)`; `je` to `ret`; else `mov (%rcx),%rax; jmp *0x4a8(%rax)` |
| Restart dispatch | 0x141ed81a0 | `cl != 0` path at 0x141ed822c: `cmp $1,%cl`; `call 0x142bcb050`; if `al == 0` and `dil != 0` (exit flag): `xor %ecx,%ecx; jmp 0x142ce8ac0` |
| Restart stub | 0x142bcb050 | `cmpb $4,0x148247e70` gate; logs `Restart application is not supported or implemented in current platform` (VA 0x1465bf8b0); `xor %al,%al; ret` at 0x142bcb07f |
| Exit | 0x142ce8ac0 | `cmpb $5,0x1482477f8` gate; logs `FPlatformMisc::RequestExit(%i)` (VA 0x1465bf830); non-forced exit tail per RESTART_CALL.md (not re-verified here) |

Modal is called with `bShouldExit` = byte +0x448 (name from `OnButtonSet` parameter `InbShouldExit` in `FortniteUI/Generated/UFortRestartClientModal.h:19`; the reflected member name for +0x448 is not confirmed here).

Vtable slots of the modal (vtable VA 0x146d700c8, from `panel_pop_stack_runtime/BEHAVIOR.md` section 0): +0x4a8 = 0x1407ce810, +0x4c0 = 0x1407e1130 (both verified in the frozen evidence).

## 2. C++ body

```cpp
// Project-local names for unreflected pieces are address-suffixed. Reflected names are used as-is.
void UFortRestartClientModal::HandleButtonClicked()
{
    // [1] 0x144f5fa02: NativeRestartPlatform141ed81a0(bRestart = true, bExit = byte +0x448).
    NativeRestartPlatform141ed81a0(/*bRestart*/ true, bNativeRestartFlag448);

    // [2] 0x144f5fa07: byte +0x304 is the activation-stack membership flag.
    if (!bNativeStackMembership304)
    {
        // Fallback, 0x1407b7ed0. Tail call through virtual +0x4a8 (NativeDeactivateRequest).
        if (bIsActive)                    // byte +0x288, verified at 0x1407b7ed0
        {
            NativeDeactivateRequest();    // virtual; Panel override 0x1407ce810 (section 4)
        }
        return;
    }

    // [3] 0x144f5fa13: virtual +0x4c0 -> 0x1407e1130 (panel pop self, section 3).
    NativePanelPop1407e1130();            // project-local name, virtual slot +0x4c0
}

// NativeRestartPlatform141ed81a0(bRestart, bExitIfNotRestarted), verified at 0x141ed81a0.
void NativeRestartPlatform141ed81a0(bool bRestart, uint8 bExitIfNotRestarted)
{
    if (bRestart)
    {
        if (FPlatformMisc::RestartApplication())   // 0x142bcb050; identical log text in engine
        {
            return;
        }
    }
    else
    {
        // Launcher branch (Mode 0). Not reached from HandleButtonClicked.
        // UNKNOWN body (section 5, U7). The binary then falls through to the exit check below.
    }
    if (bExitIfNotRestarted != 0)
    {
        FPlatformMisc::RequestExit(false);   // 0x142ce8ac0
    }
}
```

Notes on [1]. The `bRestart == false` branch is not reached from the modal, so its body is left UNKNOWN. Verified restart stub: returns `false` on its only reachable path.

## 3. Panel pop (vtable +0x4c0, 0x1407e1130)

Verified in `panel_pop_stack_runtime/BEHAVIOR.md` section 4 and `common_activatable_panel_native_events.md` section 4.1:

```cpp
void UCommonActivatablePanel::PopSelf_0x1407e1130()   // name project-local (not reflected)
{
    if (NativeInputManager.IsValid())                 // weak @+0x3b0 (declared UCommonActivatablePanel.h:90)
    {
        UCommonInputManager* Manager = NativeInputManager.Get();  // no null check in binary
        Manager->NativePopActivatable1407e0c80(this);            // PopActivatable body: NOTES.md pseudo-C
    }
}
```

## 4. Deactivate fallback (vtable +0x4a8, 0x1407ce810)

Verified in `BEHAVIOR.md` section 1 and `common_activatable_panel_native_events.md` section "Exact defaults". The gate is byte +0x303 (`cmpb $0,0x303(%rcx)`):

```cpp
void UCommonActivatablePanel::NativeDeactivateRequest_0x1407ce810()
{
    if (!bNativeFlag303) return;                     // byte +0x303 (IsIntroed state, frozen doc)
    NativeHelper1407ae640();                         // 0x1407ae640 (section 5, U21/U22)
    NativeHelper140809130();                         // 0x140809130 -> ProcessEvent(OnBeginOutro) via +0x210 (frozen doc)
    // log "Deactivate: %s" (format verified at VA 0x1459ef340, UTF-16)
}
```

## 5. UNKNOWN triage (NOTES.md U1-U28)

Classes: (A) affects observable behavior of HandleButtonClicked; (B) layout artifact, equivalent C++ given; OFF-PATH = not reached from HandleButtonClicked.

| ID | Class | Result |
|---|---|---|
| U1, U2, U3 | OFF-PATH | PushWriter 0x1407fc713 is op processing, not reached from HandleButtonClicked |
| U4 | A (log text only) | `this+0x18` identity unresolved; only in log arguments |
| U5 | B (dispatch) | Owner of slots is a naming question; virtual call is equivalent. Modal vtable 0x146d700c8 differs from panel primary vtable 0x1459edb10 (frozen doc) |
| U6 | B, CLOSED | `bIsActive` = byte +0x288. Frozen doc: setter 0x140777600 writes `[RCX+288]`; verified `cmpb $0,0x288(%rcx)` at 0x1407b7ed0 |
| U7 | OFF-PATH | Launcher branch needs Mode 0; HandleButtonClicked passes Mode 1 (verified `cmp $1,%cl` at 0x141ed822c) |
| U8 | A, CLOSED | Platform restart stub 0x142bcb050 returns false (verified). Engine `FGenericPlatformMisc::RestartApplication` (GenericPlatformMisc.cpp:666-670) same text and return; `FUnixPlatformMisc` does not override it (grep) |
| U9 | A, CLOSED | `RequestExit(false)` (verified `xor %ecx,%ecx` before `jmp 0x142ce8ac0`). Linux: `FUnixPlatformMisc::RequestExit`, UnixPlatformMisc.cpp:330-371, non-force path calls `FGenericPlatformMisc::RequestExit(Force)` |
| U10 | B | Log calls equal `UE_LOG`. Category handles and exact verbosity mapping (gate `cmpb $4` vs engine `Display`) UNKNOWN |
| U11 | A (log text) | Equivalent `FName::ToString()`; return layout UNKNOWN |
| U12 | A | Payload and op describe values are copied into queued operations (pop path); meaning UNKNOWN |
| U13 | A / OFF-PATH | `0x142ec7730` (FindFunctionChecked, used by 0x809130) and `0x142cd8ed0`, `0x1407bdba0`, `0x1407ac590` are on the fallback path (A, UNKNOWN). `0x142ee3b30` and `0x140684de0` appear in PushWriter (OFF-PATH); `0x140684de0` also precedes the exit (RESTART_CALL.md open item, A) |
| U14 | A | Enqueue helper on pop path; return ignored; semantics UNKNOWN |
| U15 | A | Operations grow and count write on pop path; exact TArray API UNKNOWN |
| U16 | A | Predicted stack copy on pop path; layout inferred |
| U17 | A | `0x1407ab920` on pop path (when `+0xe0` clear), body UNKNOWN. `0x1407b7ed0` CLOSED (verified fallback) |
| U18 | A | Operation element fields (flags 10-12, payload 14) affect queued work; `FOperation.h` has a different layout (no payload, `bIntroPanel`/`bActivatePanel`/`bOutroPanelBelow`), so the raw op is UNKNOWN |
| U19 | B, CLOSED | Empty string: VA 0x1459a4b30 first UTF-16 unit is NUL (verified) |
| U20 | B | `1480d2260` = FName `OnBeginOutro` (frozen doc: literal initializer `1401c5cc0`). `1482675e8` = log category argument, identity UNKNOWN (log-only) |
| U21 | A | Helper_ae640 fields `+0x308`, `+0x3c0..`, `+0x360` roles UNKNOWN; on fallback path |
| U22 | A | Writes `Mgr+0xa8`/`+0xb0` after unchecked Get (verified in BEHAVIOR.md 1.1); field meaning UNKNOWN |
| U23, U24, U26 | OFF-PATH | PushWriter callbacks, bind and label staging |
| U25 | OFF-PATH | `0x1408090f0` is OnBeginIntro wrapper (frozen doc); `0x1408090b0` UNKNOWN; both in PushWriter path |
| U27 | A | Modal slot +0x490 -> 0x145160190 is on the outro path (reached from +0x4b0). Target only partly described (BEHAVIOR.md 2.2) |
| U28 | B (slot numbers) / A (targets) | Slot numbers are virtual calls in C++. Verified targets: +0x4a8 = 0x1407ce810, +0x4b0 = 0x1407bb630, +0x4c0 = 0x1407e1130, +0x4d8/+0x4e8 = stub 0x1406801c0 (`ret 0`). Others remain UNKNOWN |

## 6. Discrepancies found

- The generated `UFortRestartClientModal.h` does not declare `HandleButtonClicked` (only `OnButtonSet` at line 19).
- The project body of `UCommonActivatablePanel::NativeDeactivateRequest` (`CommonUI/Generated/UCommonActivatablePanel.cpp:221-232`) does not match the verified 0x1407ce810: it uses `bNativeIsIntroed`, calls `NativeClearHeldAction()` and `OnBeginOutro()`, and a CSV event. The binary calls helpers 0x1407ae640 and 0x140809130 and logs `Deactivate: %s`, gated by byte +0x303.
- `FOperation.h` layout does not contain the raw op fields used by PopActivatable.

## 7. Pieces lacking a C++ implementation in the generated CommonUI and FortniteUI

- `UFortRestartClientModal::HandleButtonClicked`: not declared in `FortniteUI/Generated/UFortRestartClientModal.h`. The only `HandleButtonClicked` in the generated tree is `UCommonButton` (`CommonUI/Generated/UCommonButton.h:287`).
- Restart modal fields `+0x304` (stack membership) and `+0x448` (exit flag): not declared in the modal header.
- `NativeRestartPlatform141ed81a0` (project-local): no C++ present. Engine supplies `FPlatformMisc::RestartApplication` (GenericPlatformMisc.h:751) and `RequestExit(bool)` (GenericPlatformMisc.h:736).
- Panel pop self (vtable +0x4c0): not declared in `CommonUI/Generated/UCommonActivatablePanel.h`.
- `UCommonInputManager::NativePopActivatable1407e0c80`: not declared in `CommonUI/Generated/UCommonInputManager.h` (only `ActivatablePanelStack` line 29, `Operations` line 37, `IsInputSuspended` line 40). Body, predicted-stack helper, enqueue helper and operation payload UNKNOWN.
- `NativeHelper1407ae640` and `NativeHelper140809130`: not declared. `UCommonActivatablePanel::NativeDeactivateRequest` exists but differs from the verified body (section 6).
- Present and usable: `bIsActive` (`UCommonActivatableWidget.h:68`), `NativeInputManager` weak pointer (`UCommonActivatablePanel.h:90`), `OnBeginOutro` (`UCommonActivatablePanel.h:73`), `NativeDeactivateRequest` virtual (`UCommonActivatableWidget.h:106`).
