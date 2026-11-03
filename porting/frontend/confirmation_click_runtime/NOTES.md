# Confirmation click handlers: BotClickButton and FortRestartClientModal.HandleButtonClicked

Not built, not run, not committed. Scope: the two handlers only. The async CustomInput function is untouched.

## Evidence (image-125.runtime.bin, VA - 0x140000000; disassembled with objdump in small windows)
- BotClickButton: reflected exec 145218540 (flags 0x04080401). It reads +0x20 of the script frame (`Code += (Code != 0)`), then tail-jumps to body 144f54030. The body is the handler itself, so the previous `NativeBotControllerEligible144f54030` helper should be dropped.
- Restart modal: reflected exec 1452c2a80 (flags 0x04080401) tail-jumps to body 144f5f9f0. The 0x38-byte window matches the handoff hash 63cfa17dd8a189d6...
- Fallback 1407b7ed0 (verified): `cmpb 0x288(%rcx),0; je ret; jmp *0x4a8(%rax)`.
- StaticClass-style getter 142a3d9f0 (function-local cache at 1481b0238) is used for the Athena class check. Its class identity is inferred from the call site, not from a named symbol.

## Control-flow reading
- BotClickButton: GetOwningPlayer (vtable +0x288) -> null: return. Class-tree fast IsA check against Athena: fail: return. Then `(+0x1838 || +0x1818)` on the controller: false: return. Then if `window+0x570 > 0 && ConfirmationButtons.Num() > 0`, tail-call 144f5fbc0 with `ConfirmationButtons[0].Button`; else tail-call 144f5fce0 with raw `+0x4a8`.
- HandleButtonClicked: call 141ed81a0(true, +0x448); if +0x304: tail-call vtable +0x4c0; else tail-call 1407b7ed0 (returns unless +0x288, then vtable +0x4a8).

## Differences from the existing lifecycle fragment
`confirmation_window_lifecycle_runtime/definitions.inc` BotClickButton uses `ConfirmationDescription.ConfirmButtonInputActions.Num()` and `NativeDismissConfirmation(Declined)`, and reads eligibility from `NativeBotControllerEligible144f54030`. The disassembly shows the decline branch calls 144f5fce0 with `+0x4a8`, and the eligibility test is inline in the body. Keep the new version unless the owner has evidence for the old one. The `IsClientBot` declaration in that folder's members.inc is on the window, but the flags belong to the controller.

## Blockers (no fallbacks written)
1. Window +0x570 int: mapping to `ConfirmButtonInputActions` is inferred only. Needs the struct offset from the reflected layout.
2. Window +0x4a8 raw button pointer: writer and type not recovered. Declared as `UCommonButton*`.
3. Controller bytes +0x1818 / +0x1838: the producers are still unrecovered (see `confirmation_window_lifecycle_runtime/NOTES.md`). The new code reads them as `bNativeClientBotFlag1838` / `bNativeClientBotFlag1818` from that folder's controller-members.inc.
4. Window vtable +0x288 (GetOwningPlayer): the code uses `GetOwningPlayer()` by name. Verify that the slot is the base override.
5. Modal vtable +0x4c0 (shipping target 1407e1130, panel pop via manager +0x3b0) and +0x4a8 (unknown): declared as blocked dispatch hooks. They must bind to existing base overrides, not new virtuals.
6. Modal byte +0x288: owning class not confirmed. It may belong to the base panel class.
7. 141ed81a0: restart platform call with bool and byte args. Body not recovered. Per the restart HANDOFF, a portable equivalent is still needed, and it must not be dropped.
8. Byte +0x304 membership: depends on the operation queue in common_input_manager_operation_queue.md, which is not implemented in the generated runtime.

## Integration
Place `declarations.inc` into the class bodies (window private section; modal section). Remove the old BotClickButton and `NativeBotControllerEligible144f54030` declaration and body from the lifecycle fragment only after the owner approves. No generator, native, or build files were touched.
