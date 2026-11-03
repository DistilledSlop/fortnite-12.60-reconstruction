# Legacy input tick lifecycle: translation notes

Image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (VA = 0x140000000 + offset).
Method: os.pread windows dumped to scratch, `objdump -D -b binary -mi386:x86-64 --adjust-vma=<VA>`. Call targets and RIP-relative
comments were read with the correct base. No Ghidra decompile was run in this pass. No build, no run, no commit.

Files: `declarations.inc`, `definitions.inc` (this folder only).

## Verified from asm

- **Manager tick is 1407ffc50.** Secondary table 1459ed5f8 slot 0 is 1407ffc50. Slot 1 is GetTickableTickType 1407c5600.
  Receiver is manager+0x28. The PostInitProperties link (1407e1ed0) was not re-read in this pass.
- **Held-tick vtable slot is 0x28, not 0x30.** The manager tick does `jmp *0x28(%rax)` on `[manager+0xB0]`.
  Slot 0x28 is 140802440 (global) and 140802200 (panel). Slot 0x30 is 140800b70 / 140800860 and is not used by the tick.
- **NativeTickOperationSuspension is inlined in 1407ffc50.** Offsets are manager-relative (receiver+0x28):
  - `[+0xFD]` bNativeOperationInputSuspended. If 0, go to the held path.
  - `[+0xFC]` bNativeProcessingOperations, `[+0xF0]` Operations.Num, `[+0x100]` idle counter.
  - Held path: returns unless `[+0xA8]` (object word) and `[+0xB0]` (interface word) are both nonzero. Then calls `[iface+0x28](dt)`.
  - Matches the queue fragment `manager-definitions.inc:213` except D1.
- **Panel tick 140802200 (0x23b):**
  - Gate: returns if `[P+E8]==0 && F(P+F0,0) && [P+F4]==0`, where F = 142cd8ed0.
  - `P+0x30 += dt`. Then row = 14078c640(P+E8, 1459fa130). Input = 14081b480(vt[0x290](obj)). Returns if Input is null.
  - Selector B = 14081b540(Input) (first call discarded). Index C = 14081b530(Input).
    - B==1: record R+0xFC+C*0xA8 (time), R+0xF9+C*0xA8 (enabled).
    - B==2: R+0x39C / R+0x399. Otherwise R+0x54 / R+0x51.
  - Returns unless enabled byte != 0 and time > 0.
  - Idx = 1407bdba0(P+88, &Idx, P+E8). E = array[Idx] (stride 0x98) + 0x10. Returns if no entry.
  - progress = clamp(P+30 / time, 0, 1.0 at VA 1459a86f8). Broadcast 1407ac590(E, progress).
  - Returns unless `P+30 > time` (strict).
  - Complete: 140684de0(E+0x28). Popup: key E+0x64 -> 1426e3bd0; if found and registered (index vs map at popup+0x10), RequestClose 1407f17d0.
  - Commit: if `E+0x20 != 0` and `E+0x18` is non-null and `vt[0x28]()` is true, call `vt[0x50](obj, &Idx)`.
  - Then `1407ae640(obj)` (panel clear, rcx = obj = P-0x2D8).
- **Global tick 140802440 (0x3e9).** Receiver is I = obj+0x28. Order:
  1. Weak `obj+0x48` valid AND cached widget (143d5a140 result, first qword) non-null. Else clear 1407ae750 and return.
  2. Row gate at I+0x10 (DataTable), I+0x18/0x1C (name). Return if null-ish. Else no clear.
  3. `I+0x8 += dt`.
  4. Key = {row 16B at I+0x10, weak(142ee3b30 of WeakGet(I+0x20))}. Idx = 1407bd580(I+0x28, key).
  5. RowRes = 14078c640(I+0x10, table). Return if Idx == -1 or RowRes null.
  6. Weak `I+0x38` (= obj+0x60) valid. Rec = 1407c20f0(RowRes, WeakGet(I+0x38)). Return unless `Rec[0x19]` != 0 and `Rec[0x1C]` > 0.
  7. E = array base + Idx*0x90. Progress broadcast 1407ac590(E+0x20, clamp(I+8 / time)).
  8. Return unless `I+8 > time` (strict).
  9. Eligibility 1407d0160(obj, I+0x20). Return if false.
  10. Complete: 140684de0(E+0x48).
  11. If `E+0x40 != 0`, commit when `[E+0x38]` non-null and `vt[0x28]()` is true: `vt[0x50](obj_iface, &false)`.
  12. Clear 1407ae750(obj).
- **Global clear 1407ae750.** Receiver = handler object.
  - M = `obj+0x20` (OuterPrivate = manager). Return if null.
  - Reg = 14080e600(obj). Map = `*(M+0x10)`. Return unless `Reg[0x38] <= Map[0x38]` and `Map[0x30][Reg[0x38]] == Reg+0x30`.
  - Then: `obj+0x48 = weak(null)`, `obj+0x30 = 0`, `obj+0x38..0x47 = 0`.
  - `X = 142ec9900(obj, 140806b60())`. Then `M+0xB0 = X` and `M+0xA8 = obj`.
- **Subsystem Deinitialize 1407b8610.** Receiver = subsystem. `M = [sub+0x50]`. Return if null.
  - 140690620(M+0x90, sub): multicast RemoveAll(sub)-shaped.
  - Item = GUObjectArray chunk[Index>>16] + (Index&0xFFFF)*24. `Item.Flags |= 0x20000000`.
  - `0x20000000` = `EInternalObjectFlags::PendingKill = 1 << 29` (ObjectMacros.h:541).
  - No clear of +0x50 and no broadcast.
- **Data constants.** VA 1459a86f8 = float 1.0 (both clamps). VA 1459c03e0 seeds scratch arrays with count 0.
  VA 1459fa130 is a UTF-16 literal (identity UNKNOWN).
- **Scratch-array cleanup in the global tick is a no-op.** The counts are seeded to 0 and the flag at rbp-0x11 is only written 0.

## Existing fragments checked

- `global-handler-definitions.inc`, `panel-definitions.inc`, `button-definitions.inc`: no bodies for 140802440, 140802200, 1407ae750, or 1407b8610.
- `legacy-lifecycle-definitions.inc`: has SetLegacyInputEnabled and NativeHandleLegacyInputSuspensionChanged only. No Deinitialize.
  Nothing there needs changing from this pass.

## Divergences from HANDOFF.md and the queue fragment

- **D1 (queue fragment):** The asm verbose-log branch (byte VA 1480d1cd8 >= 6 -> 142c54b70) is absent from `NativeTickOperationSuspension`. The asm tick returns void.
- **D2 (HANDOFF silent):** Panel tick has an early row-null gate before the duration increment.
- **D3 (HANDOFF silent):** Panel commit is gated on `E+0x20` flag, `E+0x18` object, and `vt[0x28]`. Popup close is gated on the popup map registration check.
- **D4 (HANDOFF omits):** Global clear returns with no effect unless the handler is registered in the manager's map (14080e600). HANDOFF describes unconditional clearing.
- **D5 (asm, no null check):** Global tick reads `Rec[0x19]` after 1407c20f0 with no null check. Panel tick does not have this issue.
- **D6 (asm):** Global tick uses a second weak at obj+0x60 for the record lookup. The held owner at obj+0x48 is used for validity, key, and eligibility. HANDOFF refers to one owner.
- **D7 (asm, index):** Global clear's index check is signed-only (no negative test). Deinit's chunk math is shown for non-negative indices only.
- **D8 (asm, latent):** Global tick's commit re-reads `E+0x40` and dereferences `[E+0x38]` without a null check. If the flag changed between the checks, the asm would fault. Not expected.
- **D9 (HANDOFF confirmed):** Deinitialize matches HANDOFF. Panel and global completion orders match HANDOFF.

## UNKNOWN list

Helper addresses with unknown semantics (26):
142cd8ed0, 14078c640, 14081b480, 14081b540, 14081b530, 1407bdba0, 1407bd580, 1407ac590, 140684de0, 142ee5dd0, 142ee7880, 143d5a140, 142ee3b30, 140806b60, 142ec9900, 14080e600, 1426e3bd0, 140811c70, 1407c20f0, 1407f17d0 (name from HANDOFF only), 1407ae640 (not translated here), 140690620, 1406934e0, 142c54b70, 142bee710, 142bfd200.

Semantic UNKNOWNs (6):
- S1: role of weak at obj+0x60.
- S2: meaning of byte VA 1480d1cd8 and the log text/category.
- S3: identity of the panel Input object (vt[0x290] result) and its record layout.
- S4: popup map identity at popup+0x10.
- S5: identity of VA 1459fa130 (UTF-16 literal bytes).
- S6: whether SharedTmp (global tick) release corresponds to a known shared-ref type.

Total UNKNOWN: 32 (26 helpers + 6 semantic).

External (not counted): 1407d0160 eligibility, owned by another worker. 1407ae640 is the panel clear. The dispatch funnel 1407c8d40 is not called by these bodies.

## Not done

- No build, run, or commit. No edits outside this folder.
- `policy.py` and the held-interface registration are not changed. The fragments are not wired into the runtime.
- No engine function names assigned. All helpers are address-suffixed.
