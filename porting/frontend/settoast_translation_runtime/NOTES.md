# SetToast translation: notes

Target: `UFortFriendSubscriptionToastWidget::SetToast` (body VA `0x144d97670`, file offset `0x4d97670`).
Pseudo-C: `porting/generated/decomp-imports/Imports/SetToast__144d97670.c`. Callee: `McpFriendSub__140cd1e80.c`.
Files: `declarations.inc`, `definitions.inc`. Naming: reflected names as-is; non-reflected names carry their VA.

## VERIFIED

Evidence: `objdump -D -b binary -mi386:x86-64` on `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`.
File offset = VA - `0x140000000`.

1. Prologue and `this+0x280 = notif`: `mov %rdx,0x280(%rcx)` at `0x4d97691`.
2. Literal `"SubscriptionNudgeHold"` is ASCII at file `0x6d6e568`. Literal `L"MCP"` is UTF-16 at file `0x5a95918` (checked with `od`).
3. Name lookup: `0x142cd49c0(out, literal, 1)`, then `0x144ddd7d0(this+0x2c0, name)`, result unused (`0x4d976c0`-`0x4d976ca`).
4. Branch on `notif+0x58` (`cmp %r12b,0x58(%rsi)`, `je 0x4d97918`).
5. Action branch: vcall `+0x288` on `this`; null skips to the MCP block.
6. Membership guard (`0x4d976fc`-`0x4d9771e`): `count(L+0x38) <= count(P+0x38)` where `P = rbx+0x10`, and `P->+0x30[count(L+0x38)] == L+0x30`.
   This matches the pseudo-C exactly.
7. `0x142a53450` is called with NO argument set up. `rcx` is whatever the preceding vcall left, so a hidden receiver cannot be excluded.
8. `0x14238ead0(rbx)` (`rcx = rbx`). True selects `this+0x2a8`, false selects `this+0x2b0`.
9. `vcall +0x2e8` with receiver `this+0x2b8`.
10. `rbx+0x3138` is the sub-object. `0x1440f3f60(sub)`: false runs both binds, then `vcall +0xbd8(rbx, sub)` runs in both cases.
11. Bind helper `0x140848f70(sub, name, IE, this, delegate*)`: IE = 0 (first call, `xor r8d`) and IE = 1 (second call, `r15d`).
    The fifth argument is a pointer stored at `0x20(%rsp)`.
12. Delegate block (16 bytes): offset 0 = function address (`0x144d80480` first, `0x144d806b0` second); offset 8 = dword 0 (`mov %r12d,0x38(%rsp)`). Offset 12 is not traced.
13. MCP gate: `notif+0xd0 != 0` and `notif+0xe0 != 0` (`cmp %r12,0xd0(%rsi)`, `cmp %r12,0xe0(%rsi)`).
14. MCP name: `0x142cd4ab0(out, L"MCP", 1)`. Then vcall `+0x150` on `this` (result discarded). Then `0x140d0e9e0(name)`, then `0x140d126c0(ptr, &out)`.
15. Payload: `0x142bfd200(0, 0x20)`. The first vtable is `0x146d714e0`, `+0x10 = 0x142bad740()`, then the vtable is `0x146d71538`.
    The count `2` is a stack local (`movl $0x2,0x48(%rsp)`). NOTE: SETTOAST.md says "+0x8 is set to 2" and "payload +0x8 is set". That is WRONG.
    Payload `+0x8` is not written.
16. The argument to `0x140cd1e80` is `(out, notif+0xd0, notif+0xe0, &{payload, 2})`. The callee reads `param_4` as `*param_4` (payload) and `param_4+1` (count), so this is consistent.
17. Common tail (`0x4d97918`-`0x4d9796a`): struct `{notif+0x40, notif+0x48, int32 notif+0x50}` on the stack. `lock incl` on the `+0x8` of the `+0x48` holder. Passed as `&struct` to vcall `+0x2c8` on `this+0x2a0`. This is NOT the order SETTOAST.md implies by its labels.
18. `r15d` = 1, or 4 when `notif+0x58 != 0` (`cmovne`). Passed to vcall `+0x270` on `this+0x2b8`.
19. `vcall +0x268` on `this+0x2a8` and `this+0x2b0`, argument `notif+0x58` as a byte.

Pseudo-C only (not disassembly-checked in this pass): the display-timer payload (alloc `0x30`; `+0x8` weak ref to `this`;
`+0x10` = `0x144d7fd10`; `+0x18` = 0; `+0x28` = unique id), the SetTimer call (`0x1446278d0`, float `-1.0` and a zero byte as stack arguments),
and the timer owner selection. `+0x1c` of the payload is not written on the traced path.

## UNKNOWN

1. Virtual-slot receiver classes and return types: `+0x288`, `+0x150`, `+0x2e8`, `+0x2c8`, `+0x270`, `+0x268`, `+0xbd8`, `+0x40` (timer payload), `+0x38` (timer payload). All slot stubs are void* receivers.
2. Types of `this+0x270` (FTimerHandle candidate; size unverified), `+0x2a0`, `+0x2a8`, `+0x2b0`, `+0x2b8`, `+0x2c0`.
3. Notif field names for `+0x40`, `+0x48`, `+0x50`, `+0x58`, `+0xd0`, `+0xe0`. `+0x58` is a bool candidate for `bHasAction`, but the reflected header has no offsets. Not mapped.
4. `this+0x294` is float DisplayTime candidate. Offset not verified against reflection. Register class of the rate argument (GPR vs XMM) not verified.
5. `0x142a53450`, `0x14238ead0`, `0x1440f3f60`, `0x140d0e9e0`, `0x140d126c0`, `0x144ddd7d0`, `0x142bfd200` (alloc vs free semantics for `(ptr, 0, 0)`), `0x142bee710`, `0x142ee3b30`. Return logic and argument types not traced.
6. The `0x120` pair release after the MCP call (shared-pointer pattern). Its writer is not traced on the path where bind is skipped. Not translated.
7. The `lStack_88` temp release through vtable `+0x10`. Receiver type not recovered. Not translated.
8. `0x1446278d0` (SetTimer) stack-argument order: byte 0 vs float `-1.0`. Not verified.
9. `0x140cd1e80` body beyond the first 362 bytes (only partly traced by the existing notes). Its return value is discarded in SetToast.
10. Callback signatures (`0x144d80480`, `0x144d806b0`, `0x144d7fd10`). Not recovered.
11. Analytics `0x1417ef620` is not called by SetToast (zero call sites in the body). Not part of this translation.
12. Compiler stack-cookie check `0x145803c70` and prologue/epilogue are not translated.
