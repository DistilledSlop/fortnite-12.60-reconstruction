# SetToast: resolution pass

## Target mismatch (read first)

- The user's target is `UFortPlayerPowerRatingToast::SetToast`, body `0x144e1cd20`
  (`generated/decomp-imports/Imports/PowerToastSetToastBody__144e1cd20.c`, raw Ghidra, prototype unresolved).
- The files in this folder (`declarations.inc`, `definitions.inc`, `NOTES.md`) translate
  `UFortFriendSubscriptionToastWidget::SetToast`, body `0x144d97670`. That is a different function.
- `strict_toast_imports_runtime/NOTES.md` says the power-toast SetToast is not translated. No translation of `0x144e1cd20` exists here.
- Everything below applies to `0x144d97670` only. Nothing here resolves the power-toast body.
- Lead for the power toast (not verified as a translation): its decompile reads `+0x270` as a widget
  (passed to `FUN_144e1c1a0`), `+0x268/+0x278/+0x280/+0x288` as widgets. HideToast notes cite reflection:
  `Banner_MemberBanner` 0x270, `Text_PowerRating` 0x278, `Text_Title` 0x280, `Text_Description` 0x288, `Text_PowerDifference` 0x268.
  So `+0x270` is not a FTimerHandle in that class.

## Evidence

Disassembly: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, `objdump -D -b binary -mi386:x86-64`.
Addresses below are file offsets (VA - 0x140000000). Body range `0x4d97670`-`0x4d97b4a` (ends in `ret` at `0x4d97b4a`).

RESOLVED (verified in asm):
- R1. `this+0x294` is read as a float: `movss 0x294(%rdi),%xmm6` (`0x4d979ea`), then `movaps %xmm6,%xmm3` (`0x4d97a78`).
  It is the 4th argument of the SetTimer call in XMM3, so the register class is XMM (float), not GPR.
  `definitions.inc` uses `uint32 RateBits`, which should be a float in XMM3.
- R2. `this+0x270` is passed by address: `lea 0x270(%rdi),%rdx` (`0x4d97a67`) as the 2nd argument of `0x46278d0`
  (the SetTimer helper). The FTimerHandle candidate is consistent with this. Its size is not verified.
- R3. SetTimer stack arguments: `mov %r12b,0x20(%rsp)` (`0x4d97a7b`) = byte 0 as the 5th argument;
  `movss 0xc11d35(%rip)` then `movss %xmm0,0x28(%rsp)` (`0x4d97a5f`, `0x4d97a6e`) = float constant at file `0x59a979c` = **-1.0f** (checked by reading the bytes) as the 6th argument.
  Order: 5th = byte 0, 6th = float -1.0. The "not verified" note in `declarations.inc` is resolved.
- R4. The "+0x120 8-byte pair" is NOT a shared-pointer slot. The only accesses to `0x120(%rsp)` are
  `movaps %xmm6,0x120(%rsp)` (`0x4d979bc`) and `movaps 0x120(%rsp),%xmm6` (`0x4d97a94`), around the SetTimer call.
  This is a 16-byte callee-saved `xmm6` spill and reload. No lock op or pointer use touches it.
- R5. `+0x1c` is not written. No instruction in the body window has displacement `0x1c` with any base register.
  `definitions.inc` line 150 should say "not written (verified)".
- R6. S19 is a CALL, not a tail call: `mov %rdi,%rcx` (`0x4d97b29`), `call 0x5238060` (`0x4d97b2c`), then a stack-cookie check
  (`call 0x5803c70` at `0x4d97b38`, `xor %rsp`) and the epilogue (`add $0x130,%rsp`, pops, `ret` at `0x4d97b4a`).
  Return value is discarded. `this` is passed in rcx.
- R7. Release of the pointer at `0x38(%rsp)` (`0x4d978d4`-`0x4d9790d`, after the MCP call `0xcd1e80` at `0x4d978cf`):
  `lock xadd -1` on `+0x8`; if the old value is 1, `call *(%rax)` (vtable +0); then `lock xadd -1` on `+0xc`;
  if the old value is 1, `call *0x8(%rax)`. This is the strong/weak release pattern of a shared-reference controller.
  Matching it to a named engine function (SharedPointerInternals.h) is NOT done here.

NOT RESOLVED (open):
- U1. `this+0x2a0`, `+0x2a8`, `+0x2b0`, `+0x2b8`, `+0x2c0`: types not recovered.
- U2. `+0x270` size and type (R2 gives only the address use).
- U3. Virtual slots `+0x288`, `+0x150`, `+0x2e8`, `+0x2c8`, `+0x270`, `+0x268`, `+0xbd8`, `+0x40`, `+0x38`: receiver classes and return types not mapped to UE4.25 virtuals.
- U4. Notif fields `+0x40`, `+0x48`, `+0x50`, `+0x58`, `+0xd0`, `+0xe0`: no reflected names or offsets checked. (`+0x58` is read as a byte, `cmp`/`cmov`, in R-trace, not named.)
- U5. `this+0x294` name (DisplayTime candidate): offset not checked against reflection.
- U6. Helpers `0x142a53450`, `0x14238ead0`, `0x1440f3f60`, `0x140d0e9e0`, `0x140d126c0`, `0x144ddd7d0`, `0x142bfd200`, `0x142bee710`, `0x142ee3b30`, `0x142cd49c0`, `0x142cd4ab0`, `0x1446278d0` (SetTimer): semantics and return types not traced.
- U7. Callback signatures `0x144d80480`, `0x144d806b0`, `0x144d7fd10`.
- U8. `0x140cd1e80` body beyond the first 362 bytes.
- U9. The `lStack_88` temp release: the R7 pattern is seen, but mapping it to `lStack_88` (pseudo-C `SetToast__144d97670.c`) is not checked.
- U10. Stack cookie `0x5803c70` is identified as the check, but not translated.

UNKNOWN count left (open items above): 10 (U1-U10). Several sub-items inside U1, U3, U6 are unverified.
No `definitions-final.inc` or `declarations-final.inc` written: the SetToast path is not fully verified.
