# UFortFriendSubscriptionToastWidget::SetToast: control-flow reading

Status: analysis only. No implementation was written. `friend-settoast-declaration.inc` stays declaration-only.

## Evidence and conventions

- Body: VA `0x144d97670`, 1243 bytes, flags per the `.inc` header. Exec stub `0x14523d570` (128 bytes) was not traced.
- Bytes: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`. File offset = VA - `0x140000000`. The SHA-256 of the SetToast body, the MCP entry `0x140cd1e80`, and the analytics entry `0x1417ef620` match `porting/frontend/strict_toast_imports_runtime/original-windows.json`.
- The task named `.../original-late-capture-1/original-windows.json`. That file does not exist; the copy in the porting folder was used.
- Addresses shown without the `0x14` image base (e.g. `0x4d80480`, `0x6d6e568`, `0x59a979c`) are file offsets in the image; add `0x140000000` to get VAs (e.g. `0x2cd49c0` = VA `0x142cd49c0`).
- Labels: **named-verified** = a name comes from a window label, a string literal, or a checked byte pattern; **named-inferred** = name from signature/usage pattern only; **unnamed** = no name found; **verified** / **inferred** apply to the behavior described.
- Reflection JSON: the three `SetToast` candidate records are marked "ambiguous: candidates disagree on parameter table", and their `create_func` values (`0x1452356b0`, `0x1452a1370`, `0x145320370`) do not match the exec `0x14523d570`. No callee in this body appears in the reflection JSON by address. Generated `UFortFriendSubscriptionToastWidget.h` declares neither `SetToast` nor `NativeToastNotification`.

## Callee table

| Callee (VA) | Label | Status | Role in SetToast |
|---|---|---|---|
| `0x140cd1e80` | MCP friend subscription operation entry | named-verified (window label) | Called once from the action branch (`0x4d978cf`) with the notification's `+0xd0`/`+0xe0` fields. |
| `0x1417ef620` | toast analytics entry | named-verified (window label) | **Not called by SetToast.** See the analytics section. |
| `0x145238060` | none | unnamed | Last call before return, with `this`. |
| `0x145803c70` | none | named-verified (stack-cookie pattern) | `__security_check_cookie`-style exit check. |
| `0x142cd49c0` | none | named-inferred (FName from literal) | Builds a name from a string literal (`0x6d6e568`, ASCII `SubscriptionNudgeHold`). |
| `0x142cd4ab0` | none | named-inferred (name/string from literal) | Builds a name from a wide literal (`0x5a95918`, UTF-16 `MCP`). |
| `0x144ddd7d0` | none | unnamed | Lookup on `this+0x2c0` by name; result is discarded. |
| `0x142a53450` | none | unnamed | Returns an object used by a validity/membership guard. |
| `0x14238ead0` | none | unnamed | Bool predicate on an object; selects `this+0x2a8` or `this+0x2b0`. |
| `0x1440f3f60` (`0x1440f3f60`) | none | unnamed | Predicate on a sub-object: `cmpl $0` on `+0x118`, `+0xd8`, `+0xe8` (any > 0 returns true; full return logic not traced). |
| `0x140848f70` | none | named-inferred (UInputComponent::BindAction-like) | Two calls with `IE 0` and `IE 1` and delegates `0x4d80480` / `0x4d806b0`. |
| `0x142ee3b30` | none | named-inferred (weak-object assignment) | Writes a weak reference to `this` into a payload. Reads the object serial at `+0xc`. |
| `0x142bfd200` | none | named-inferred (heap allocation) | Allocates a 0x20 or 0x30 byte payload; the vtable is then written. |
| `0x142bad740` | none | unnamed (atomic counter) | `lock xadd` on a global counter. Used as a unique ID in payloads. |
| `0x142bee710` | none | unnamed (release helper) | Null-checked release of a payload. |
| `0x140d0e9e0` | none | unnamed | Returns a pointer; null is a skip. |
| `0x140d126c0` | none | unnamed | Writes an out-value to `rsp+0x30`. |
| `0x1446278d0` | none | named-inferred (FTimerManager::SetTimer) | Argument pattern matches SetTimer, `FirstDelay = -1.0` (`0x59a979c` = `0xbf800000`). |

Virtual slots (none named; all unnamed):

| Slot | Receiver | Use |
|---|---|---|
| `+0x288` | `this` | Returns the object `rbx` used by the action branch. |
| `+0x2e8` | object at `this+0x2b8` | Called with a `this+0x2a8` or `this+0x2b0` sub-object. |
| `+0xbd8` | `rbx` | Called with `(rbx, r14)` after the bind step. |
| `+0x2c8` | object at `this+0x2a0` | Called with a local shared-pointer copy of `notif+0x40/+0x48`. |
| `+0x270` | object at `this+0x2b8` | Called with `r15b`. Inferred SetVisibility; values 1 (Collapsed) or 4 (SelfHitTestInvisible). |
| `+0x268` | objects at `this+0x2a8` and `this+0x2b0` | Called with `bHasAction`. |
| `+0x150` | `this` | Returns `W`, the owner used for the timer path. Also called once on `this` in the MCP block, with the result discarded. |

Other virtuals used in helpers: `+0x28`, `+0x20`, `+0x18`, `+0x30`, `+0x38`, `+0x40`, `+0x10`, `+0x8`, `+0x0` (unnamed).

## Step-by-step control flow

Inputs: `rcx = this`, `rdx = ToastNotification (notif)`.

S0. Prologue. Stack cookie (`0x80c4288` xor `rsp`), frame `0x130`.

S1. `this+0x280 = notif` (verified, `mov %rdx,0x280(%rcx)`). Matches the `NativeToastNotification` note in the `.inc`.

S2. Build name `"SubscriptionNudgeHold"` (literal `0x6d6e568`) via `0x142cd49c0`. Call `0x144ddd7d0(this+0x2c0, name)`. Result unused (unnamed lookup).

S3. If `notif+0x58` (bHasAction) is zero, jump to S12 (common tail).

S4 (action branch). `rbx = this->vcall(+0x288)`. If null, go to S10.

S5. `0x142a53450(rbx)` returns an object. A membership guard follows: `rbx+0x10` points to an object with an array at `+0x30` (count at `+0x38`). The object returned by `0x142a53450` is compared against that array. Failure goes to S10. Interpretation is inferred.

S6. `0x14238ead0(rbx)` returns a bool. If true, `rdx = this+0x2a8`; else `rdx = this+0x2b0`.

S7. `rcx = *(this+0x2b8)`; `vcall(+0x2e8)(rcx, rdx)`. Unnamed.

S8. `r14 = rbx+0x3138`. If null, go to S10.

S9. If `0x1440f3f60(r14)` is false, bind: `0x140848f70(r14, FName, IE, this, delegate)` twice. First with `IE=0` and `0x4d80480` ("friend hold pressed", verified label). Second with `IE=1` and `0x4d806b0` ("friend hold released", verified label). Then `rbx->vcall(+0xbd8)(rbx, r14)`. Skip-bind path is possible when the predicate is true. Interpretation of `IE` values is inferred.

S10 (join, `0x4d97811`). MCP block. Requires `notif+0xd0 != 0` and `notif+0xe0 != 0` (verified tests). Then:
- `rbx = 0x142cd4ab0(&tmp, L"MCP", 1)`.
- `vcall(this, +0x150)`; result discarded.
- `ptr = 0x140d0e9e0(rbx)`; null means skip. `0x140d126c0(ptr, &out)`; `out` null means skip.
- Payload: `0x142bfd200(0, 0x20, 0)`. Vtable `0x6d714e0`. `+0x10 = 0x142bad740()` (ID). Vtable `0x6d71538`. `+0x8` is set to 2. Stored at `rsp+0x40` (inferred ref-counted payload).
- Call `0x140cd1e80(rcx=out, rdx=notif+0xd0, r8=notif+0xe0, r9=&payload)` (verified call; semantics inferred from the callee entry).
- Callee entry (first 362 bytes only): calls `0x140d5dd20` with `dl=0x12`; `vcall(+0x28)` on `rdx` and `r8` as bool checks; uses `this+0x598` with `0x140a07490` (find) and `0x140aab650` (add); returns via `0x140c840b0`. Remainder not traced.
- Ownership: the payload is not released in SetToast. Assumed transferred to the callee (inferred).
- Release sequence at `0x4d978d4`: UE4 shared-pointer release pattern (`lock xadd -1` at `+0x8`, then vtable[0]; `+0xc`, then vtable[8]). It is guarded by `rsp+0x38`, which is only ever written as 0. On the traced paths this block is effectively dead (verified by writes, inferred semantics).

S11. Fall through to S12.

S12 (common tail, `0x4d97918`). Copies `notif+0x40` and `notif+0x48` into a local pair `rsp+0x50/0x58`; `lock incl` on `+0x8` if non-null (ref-count copy; inferred TSharedPtr). Copies `notif+0x50` (int) to `rsp+0x60`.

S13. `vcall(this+0x2a0, +0x2c8)(rcx, &pair)`. Unnamed.

S14. `r15d = 1`; if `notif+0x58 != 0`, `r15d = 4`. `vcall(this+0x2b8, +0x270)(r15b)`. Inferred SetVisibility.

S15. `vcall(this+0x2a8, +0x268)(bHasAction)`; `vcall(this+0x2b0, +0x268)(bHasAction)`. Unnamed; likely a paired enable/show toggle (inferred).

S16. `W = vcall(this, +0x150)`. Null skips S17.

S17 (timer). `rsi = (W->+0x188) ? (W->+0x188)->+0xc0 : W->+0x320` (inferred timer-manager lookup). `xmm6 = this+0x294` (float rate; inferred `DisplayTime`).
- Payload `0x142bfd200(0, 0x30, 0)` with vtable `0x59a9e80`. `+0x8` is a weak ref to `this` via `0x142ee3b30`. `+0x10 = 0x4d7fd10` ("friend display timer callback", verified label). `+0x28 = 0x142bad740()`.
- Delegate built into `rsp-0x50`; `vcall(rbx, +0x40)(rbx, &delegate)`. Unnamed.
- `0x1446278d0(rsi, this+0x270 (FTimerHandle), &delegate, rate=xmm6, bLoop=0, FirstDelay=-1.0)`. Inferred SetTimer.
- Cleanup: `vcall(rbx, +0x38)` and `0x142bee710(rbx)`; delegate cleanup.

S18. Cleanup of the pair and the `rsp+0x50` values (ref-count release).

S19. `0x145238060(this)`. Unnamed.

S20. Stack cookie check (`0x145803c70`); return.

## Analytics `0x1417ef620`

- Not called by SetToast: zero call sites in the SetToast body.
- Direct `E8` scan of the whole image finds three callers: `0x4d7fddd` (inside the friend display timer callback `0x4d7fd10` window), `0x4d80812` (inside the friend hold released window `0x4d806b0`), and `0x4d9b2c8` (outside the friend window, unnamed).
- Conclusion (verified): in this set, analytics is reached from the timer and hold-released callbacks, not from SetToast.

## Open items

1. Contract of `0x140cd1e80`: beyond the first 362 bytes, the operation body and its payload handling are not traced.
2. Virtual slot identities (`+0x288`, `+0x2e8`, `+0xbd8`, `+0x2c8`, `+0x268`, `+0x150`) need the concrete classes. No class layout was resolved.
3. Field names at `this+0x2a0`/`+0x2a8`/`+0x2b0`/`+0x2b8`/`+0x2c0`/`+0x294`/`+0x270`, `notif+0x40/+0x48/+0x50/+0x58/+0xd0/+0xe0`, and `rbx+0x3138`: no names found in the generated headers. The `.inc` only names `NativeToastNotification` at `+0x280`.
4. `0x142a53450`, `0x14238ead0`, `0x1440f3f60`, `0x140d0e9e0`, `0x140d126c0`: named by usage only; their return logic is not fully traced.
5. Analytics call contract (argument list and category) is not recovered from its callers in this pass.
