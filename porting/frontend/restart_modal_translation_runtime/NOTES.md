# restart_modal_translation_runtime: notes

Scope: translation of the restart-modal click path from `porting/generated/decomp-imports/Imports/*.c`.
Files: `declarations.inc`, `definitions.inc` (fragments, not compiled or registered). Nothing built or run.

## Sources and verification

| Function (VA) | Pseudo-C file | Objdump cross-check (image-125, file offset = VA - 0x140000000) |
|---|---|---|
| HandleButtonClicked (0x144f5f9f0) | RestartClick__144f5f9f0.c | Verified: `movzbl 0x448(%rcx)`, `cmpb $0,0x304(%rbx)`, `jmp *0x4c0(%rax)` / `jmp 0x7b7ed0`. Matches pseudo-C. |
| NativeRestartPlatform (0x141ed81a0) | RestartPlatform__141ed81a0.c | Not re-disassembled. Pseudo-C only. |
| NativePanelPop (0x1407e1130) | PanelPop__1407e1130.c | Via BEHAVIOR.md (weak IsValid/Get). Pseudo-C only. |
| NativePopActivatable (0x1407e0c80) | PopActivatable__1407e0c80.c | Verified offsets: 0xb8, 0xc0, 0xe0, 0xe8, 0xf0, 0xf4. Rest pseudo-C only. |
| NativePushWriter (0x1407fc713) | PushWriter__1407fc713.c | Verified: `movb $1,0x304(%rbx)`, `call *0x4e8(%rax)` with RCX not set in the window, `mov %rbx,%rcx; call 0x8090b0`, `lea 0x48(%r14),%rcx; call 0x684de0`. |
| NativeOutroDefault (0x1407bb630) | OutroDefault__1407bb630.c | Via BEHAVIOR.md section 2 (RDX forwarded; slot 0x4d8 is a `ret 0` stub). |
| NativePanelOutroReq (0x1407ce810) | PanelOutroReq__1407ce810.c | Via BEHAVIOR.md section 1 (`cmpb $0,0x303(%rcx)`, format `Deactivate: %s`). |
| NativeHelper1407ae640 (Helper_ae640) | Helper_ae640__1407ae640.c | Via BEHAVIOR.md section 1.1. Pseudo-C only otherwise. |
| NativeHelper140809130 (Helper_809130) | Helper_809130__140809130.c | Via BEHAVIOR.md section 1.2 (thunk, slot +0x210). |

Names: reflected names are used as-is (`HandleButtonClicked`, `ActivatablePanelStack`, `Operations`, `bIsActive`, `bNativeRestartFlag448` is project-local). All other functions, members and globals use VA-suffixed names. Callees whose semantics the pseudo-C does not establish use the `NativeCall<VA>` prefix.

## Discrepancies with existing files (not changed here)

1. `HandleButtonClicked` is not declared in the generated `UFortRestartClientModal.h` (only `OnButtonSet`). The only generated `HandleButtonClicked` is in `CommonUI/Generated/UCommonButton.h`. The reflected exec `1452c2a80` comes from HANDOFF.md, not from the header.
2. `panel_pop_stack_runtime/panel-pop-definitions.inc` breaks the pop loop when the helper's return equals the panel. The pseudo-C breaks when the loop entry equals the panel and ignores the helper's return. This translation follows the pseudo-C.
3. The byte at +0x304 is named `bNativeInActivationStack` in `panel_pop_stack_runtime` and `bNativeStackMembership304` here. Same byte, different name.
4. The redundant-pop log in `PopActivatable` says `InternalPushActivatablePanel`. Kept verbatim.
5. `NativeRestartPlatform` callee `0x142ce8ac0` is labelled `FPlatformMisc::RequestExit` in HANDOFF.md. That label is not verified in this pass, so the name here is neutral.

## UNKNOWN items

Entry state and identity
- U1. `NativePushWriter1407fc713` is a mid-function fragment. The function start is not established (WRITERS.md). Parameter identities for `RBX`, `R14`, `[RBP-0x31]`, `RDI`, `[RBP-0x28]`, `[RBP-0x29]` and the `RBP-0x25` payload are UNKNOWN.
- U2. The receiver RCX for the `call *0x4e8` at 0x7fc71a is not set inside the verified window. The translation passes `ObjRbx`, which is an assumption.
- U3. `[RBP-0x31]` and `RBX` are both treated as panel-class objects. Whether they are the same object is not verified. WRITERS.md also reports a TArray at `(%rbx)` in the push loop, which conflicts with `RBX` being the panel.
- U4. `this+0x18` (read by PanelOutroReq, PopActivatable, PushWriter, and passed to `NativeCall142ce9f90`): field identity UNKNOWN (BEHAVIOR.md).
- U5. Owner class of `NativeOutroDefault1407bb630`, `NativePanelOutroReq1407ce810`, `NativeHelper*`, and `NativePanelPop1407e1130` is assumed to be `UCommonActivatablePanel`. The vtable `0x146d700c8` is the restart modal's primary vtable (HANDOFF), so the owner of these slots is not confirmed.
- U6. Byte `+0x288` is taken as reflected `bIsActive` (offset from HANDOFF and pseudo-C). Reflection offset not re-checked.

Callees and globals (semantics or signature UNKNOWN)
- U7. `NativeCall142cd49c0`, `NativeCall142cbd480`, `NativeCall142cc0600`, slot +0x48 on the launcher object (launcher branch, Mode 0 only).
- U8. `NativeCall142bcb050` (platform restart check). HANDOFF says it logs and returns 0 on the captured platform. Not verified here.
- U9. `NativeCall142ce8ac0(0)`: role UNKNOWN (see discrepancy 5).
- U10. `NativeLog142c54b70`, `NativeLog142cc40c0`: the variadic/format signatures are inferred from the pseudo-C call shape only.
- U11. `NativeCall142ce9f90`: FName-to-string role inferred from usage. Return shape (FString pointer with data at +0 and count at +8) is inferred from the pseudo-C reads.
- U12. `NativeCall142ca88a0` (op describe), `NativeCall142c9ff10` (16-byte payload): semantics UNKNOWN. The payload values are not interpreted.
- U13. `NativeCall142ec7730`, `NativeCall142ee3b30`, `NativeCall142cd8ed0`, `NativeCall1407bdba0`, `NativeCall1407ac590`, `NativeCall140684de0`: semantics UNKNOWN.
- U14. `NativeCall1407a95e0` (op enqueue helper): name is a neutral label. Return value is ignored by the pseudo-C.
- U15. `NativeGrow14069ea30` and the TArray count write for `Operations` (+0xf0) during the intro append: the grow call is kept. The count write is shown only as a comment, since the exact TArray API is UNKNOWN.
- U16. `NativeCall1407ad3f0` (predicted stack copy): output array type and layout inferred only from the pseudo-C.
- U17. `NativeCall1407ab920` (begin op processing), `NativeCall1407b7ed0` (fallback): bodies not in scope.
- U18. Operation element fields: `NativeOpFlag10/11/12` are set to 1 in both writers. `NativeOpFlag13` and `NativeOpPad24` are never set by the pseudo-C. `NativeOpPayload14[4]` meaning UNKNOWN. Only Intro = 0 and Pop = 3 are verified as operation values.
- U19. `NativeEmptyString1459a4b30`: the content is assumed to be `""` from its use as a fallback. Not verified.
- U20. `NativeGlobal1482675e8` (first arg of the Deactivate log) and `NativeGlobal1480d2260` (argument to `NativeCall142ec7730`): roles UNKNOWN.
- U21. `NativeHelper1407ae640` fields: `NativeFloat308`, `NativePtr3C0`, `NativeInt3C4`, `NativeInt3C8`, `NativeInt3CC`, `NativeArray360` (element stride 0x98, element struct UNKNOWN). Roles are inferred from usage only.
- U22. `NativeHelper1407ae640` writes `Get()+0xa8` and `Get()+0xb0` without a null check (verified in BEHAVIOR.md). The meaning of these two fields is UNKNOWN.
- U23. `NativeCallback3D0` and `NativeCallback3E0` (bind targets in PushWriter) and the staging records at `[RBP-0x59]`, `[RBP-0x51]`, `[RBP-0x25]`: types and layout UNKNOWN. Several staging stores in the pseudo-C (`[RBP-0x55]`, `[RBP-0x4d]`, `[RBP-0x79]`, `[RBP-0x71]`, `[RBP-0x6d]`) are not reproduced.
- U24. `NativeCall140786850` (bind), `NativeCall140680160`, `NativeCall1406800e0`, `NativeCall140789600` (scratch-buffer build for the label): semantics UNKNOWN. The label scan loop is translated, the buffer build is not.
- U25. `NativeCall1408090b0` and `NativeCall1408090f0`: names unknown (WRITERS.md).
- U26. PushWriter log arguments for the op text and the label text are passed as `NativeEmptyString1459a4b30` placeholders. They are UNKNOWN in the pseudo-C and must be replaced when the describe and label values are recovered.
- U27. `NativeOutroDefault1407bb630` forwards the entry RDX to the tail call at `0x490`. RDX type is UNKNOWN and absent from the pseudo-C signature. Slot `0x490` targets `0x145160190`, which BEHAVIOR.md describes only partially.
- U28. `NativeVtblCall` slot targets (0x48, 0x210, 0x488, 0x490, 0x4c0, 0x4d8, 0x4e8) are verified as slot offsets. Their target functions are not all verified here. Argument types at each call site are inferred from the pseudo-C.
