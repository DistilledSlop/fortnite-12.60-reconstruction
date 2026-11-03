# SetTriggeringInputAction: notes and blockers

Original: 1407fa9e0, 95 bytes. Verified against image-125.runtime.bin
(`/mnt/vibrato/fortnite-evidence/original-late-capture-1/`), VA-base 0x140000000.
Helper 14418a080 (38 bytes) verified too. Disassembled with objdump.

## Recovered behavior
- 14418a080(existing, incoming) returns 0 only when DataTable ptr, FName Number and
  ComparisonIndex all match. Setter returns on 0.
- Otherwise: teardown 140801750(this), copy 16 bytes from incoming to +2b0, virtual +488
  (NativeBindTriggeringInputAction), virtual +4a0 (NativeUpdateActionWidget).
- Corrects the older common_button_public_producers_runtime/HANDOFF.md, which said a
  non-null old row returns early. It does not.

## Blockers
1. Dependencies are gated. NativeUnbind/NativeBind bodies live in
   strict_commonui_imports_runtime/button-definitions.inc, which policy.py does not
   enable (needs the legacy held-input interface/manager closure). This setter is
   source-only until those are enabled.
2. Duplicate definition. button-definitions.inc already defines
   UCommonButton::SetTriggeringInputAction with the same body. Integrate only one copy.
3. Generated header gap. porting/native/Source/CommonUI/Generated/UCommonButton.h has
   only the TriggeringInputAction member, with no SetTriggeringInputAction
   declaration, and UCommonButton.cpp has no definition. Root must reconcile this
   (read-only for this task).
4. Not compiled or run. This is static source recovery only. No build or game run.
5. Helper 14418a080 is written inline as FDataTableRowHandle operator==, not as a
   named function. Equivalent result, unnamed in the binary.
