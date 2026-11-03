# Legacy input eligibility (VA 0x1407d0160) and helper (VA 0x1407cf620): translation notes

Image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`.
File offset = VA - 0x140000000. Disassembly: `objdump -D -b binary -mi386:x86-64` over
0x7d0160-0x7d0660 (function ends at 0x7d0657). Ghidra cross-check:
`porting/generated/decomp-imports/Imports/Eligibility1407d0160__1407d0160.c` and
`.../NativeHelper1407cf620__1407cf620.c`. Ghidra parameter recovery was off; the
translation follows the asm, and the decompile was used only as a cross-check.

Files: `declarations.inc`, `definitions.inc` (this folder). Not built, not run, not enabled by any policy.

## Verified facts

### Entry and signature
- 1407d0160-1407d0185: `rcx` is overwritten by `mov %rdx,%rcx` before any read. Only `rdx`
  (row input record) is live. Signature is `bool(void* UnusedRcx, void* RowInputRecord)`.
- Caller 140802440 at 80262b/80262f: `lea -0x28(%rdi),%rcx`, `lea 0x20(%rdi),%rdx`, `call 0x7d0160`.
- Return value is `bl`, moved to `al` at 1407d063a (`movzbl %bl,%eax`). The early `xor %al,%al` at
  1407d028b also returns false. The loop-completion path sets `bl=1` at 1407d0465.

### Helper 1407cf620 (fully verified by asm 1407cf620-1407cf670)
- Order: call 143d60da0(rcx) first (result in bl), then 143d5cb70(rdi).
- Returns false if GetIsEnabled is false, if `byte[+0x2a1] & 6 != 6`, or if `(byte)(bl-1) <= 1`
  (asm `dec %bl; cmp $1,%bl; jbe`). Otherwise true.

### GetIsEnabled 143d5cb70 (structure verified)
- Calls 143d5a140 with rdx = out buffer. Object at out+0, control at out+8 (3d5cb87, 3d5cb8c).
- Object non-null: reads attribute at object+0x190 (byte cache). Live if qword `[+0x1a0] != 0`,
  ptr `[+0x198] != 0`, and `vtbl[0x28]()` true. If live and dword `[+0x1a0] != 0`, then
  `*(+0x190) = vtbl[0x48](ptr)`. Returns byte `[+0x190]`.
- Object null: returns `(byte[widget+0xb4] >> 2) & 1`.
- Releases out control if non-null (3d5cbe6-3d5cc13).
- Source match: `Runtime/UMG/Private/Components/Widget.cpp:242`
  (`GetCachedWidget().IsValid() ? IsEnabled() : bIsEnabled`). The fallback bit is `bIsEnabled`
  at byte +0xb4 bit 2 (asm). The UMG bitfield position was not confirmed.

### Shared control block (layout from asm)
- +0x8 int32 shared count, +0xc int32 weak count, vtbl[0] = dispose, vtbl[1](ptr,1) = delete.
  Release sequence: 1407d02f9-1407d0318, 1407d0605-1407d0625, 3d5cbf0.
- AddRef: `incl 0x8(ctrl)` at 1407d032b and 1407d035a.

### Main flow of 1407d0160 (each branch checked against asm)
1. 7d018b: `r = Lookup(rdx)`. 7d0190: `GetIsEnabled(r)`. 7d019a: jne -> enabled path (7d01d2).
2. Not enabled (7d019c): if `byte[0x80d1df0] < 6` return false. Else log
   "Global Input (Not Enabled, Ignoring): %s" (fmt 0x59f2530, arg = Lookup(rdx)+0x18 via 142ce9f90)
   at 7d01a9-7d024a, then return false.
3. Enabled path (7d01d2-7d0292): `owner = Lookup(rdx)`; if null -> general path.
   - `ctx = 140807cd0()`; `list = owner+0x10`; `idx = movslq ctx+0x38`.
   - Passes if `idx <= [list+0x38]` (jg at 7d01fa) and `list[+0x30][idx] == ctx+0x30` (7d0204-7d0208).
   - If both pass and `helper1407cf620(owner)` is false: verbosity gate (7d021a), then log
     "Global Input (Interaction Not Enabled, Ignoring): %s" (fmt 0x59f2590), return false.
   - Any failed test falls to the general path (7d0292).
4. General path (7d0292-7d0629):
   - Local path ctor 142f44d90(path). `g = [0x8271318]`. `ret = 143d5a140(Lookup(rdx), &sret)`.
   - Pinned pair `(ret[0], ret[1])` is copied, then `ret[0]` and `ret[1]` are zeroed (7d02cf-7d02de).
     Then sret+8 is released (7d02e5-7d0318). Its relation to `ret` is UNKNOWN (U20).
   - AddRef pinned ctrl (7d0326-7d032b). Save pair in xmm6 (7d031b).
   - `14300d060(g, &pinned, path, byte[0x7c522f0])` (7d032e-7d0342). If false -> 7d057d.
   - On true: restore pair and AddRef again (7d034f-7d035a). `142f4c5b0(path, &pinned)` (7d0366);
     if false -> cleanup with false (7d05ee).
5. Element loop (7d0373-7d0457). Base = `path+0x128 ? path+0x128 : path+0x8` (7d0373-7d037e).
   Count = int32 `path+0x130`. End = base + count*0x48 (7d038d-7d0398).
   Per element `e`:
   - `w = [e+0x38]`.
   - Attr1 = `w+0x190`: live test as above. If live: `[+0x190] = vtbl[0x48]()`, with no out-pointer
     argument (7d03ec-7d03f2).
   - If `[+0x190] == 0` (7d03f4 je 7d04ce): verbosity gate (7d04d5, `jb` to 7d0571). Otherwise log
     "Global Input (%s !IsEnabled), Ignoring: %s" (fmt 0x59f2600). Args: widget name via 142fc3fe0
     (7d04b7) and record name via 142ce9f90 (7d0499-7d04b1). Then break, return false.
   - Attr2 = `w+0x1a8`: live test with `+0x1b8`/`+0x1b0`. If live: `[+0x1a8] = vtbl[0x48](&out)` (byte out,
     7d0439-7d0449).
   - If `([+0x1a8] & 0x10) == 0` (7d044b-7d046c): verbosity gate, then log
     "Global Input (%s !AreChildrenHitTestVisible), Ignoring: %s" (fmt 0x59f2660). Then break, return false.
   - Otherwise `e += 0x48`. Loop completion returns true (7d045d-7d0467).
6. Not found (7d057d): verbosity gate (7d0584 `jb` to 7d05ee). Log "Global Input (No Path To Widget): %s"
   (fmt 0x59f26e0) with the record name. Return false.
7. Cleanup (7d05f0-7d0657): release the pinned ctrl saved in rsi, call 14079aa60(path), return bl.

### Log details (verified from asm and image strings)
- Log emitter 142c54b70 is called as `(0, 0, &0x80d1df4, 6, fmt, arg1[, arg2])`.
- Verbosity gate: `byte[0x80d1df0]` compared unsigned against 6. In `ELogVerbosity`
  (`Runtime/Core/Public/Logging/LogVerbosity.h`: NoLogging=0, Fatal, Error, Warning, Display, Log,
  Verbose=6, VeryVerbose), 6 is `Verbose`. The message verbosity is also 6 (r9d).
- Format strings (UTF-16 read from image):
  - 0x59f2530 `Global Input (Not Enabled, Ignoring): %s`
  - 0x59f2590 `Global Input (Interaction Not Enabled, Ignoring): %s`
  - 0x59f2600 `Global Input (%s !IsEnabled), Ignoring: %s`
  - 0x59f2660 `Global Input (%s !AreChildrenHitTestVisible), Ignoring: %s`
  - 0x59f26e0 `Global Input (No Path To Widget): %s`
  - 0x59a4b30 is the empty string L"".

### Local path object (offsets verified only)
- Base = path. Inline element storage at +0x8 (4 x 0x48 bytes), heap data pointer at +0x128,
  int32 count at +0x130 (asm 7d0373-7d038d, using rbp-0x80 as path base).

### Slot reuse in log paths (asm)
- In the "!IsEnabled", "!AreChildren", and "No Path" log paths, the record-name FString is written to
  the same stack slot as the pinned pair (rsp+0x50/0x58). The C++ uses a separate `LegacyFStringTemp`.
  After those paths the cleanup uses the register copy of the ctrl (rsi) only, so the result is the same.
  The object-pointer reference in that slot is not released in those paths. Ownership consequences were not analyzed.

## UNKNOWN list (22 items)

Each UNKNOWN is left as address-suffixed or descriptive names. No engine name is claimed.

- U1  142ee5dd0: record resolver body (called at 7d018b, 7d01d5, 7d02bd, 7d0223, 7d047c, 7d04de, 7d0589). Only +0x10 and +0x18 uses are verified.
- U2  140807cd0: context object body (7d01e6). Only +0x30 (array ptr) and +0x38 (index) uses are verified.
- U3  142f44d90: local path constructor (7d02ae). Size and fields UNKNOWN.
- U4  14079aa60: local path destructor (7d062d).
- U5  14300d060: path finder, bool (7d0342). Arguments as passed in the asm; semantics UNKNOWN.
- U6  142f4c5b0: owner-in-path check, bool (7d0366). Semantics UNKNOWN.
- U7  142fc3fe0: widget name to string (7d04b7, 7d0519). Semantics UNKNOWN.
- U8  142ce9f90: name value to FString (7d01c4, 7d0494, 7d04f6, 7d059c). Semantics UNKNOWN.
- U9  142c54b70: log emitter. Identity (FMsg::Logf or similar) not verified.
- U10 142bee710: free (7d0286, 7d055d, 7d056c, 7d05e9). Name not verified as FMemory::Free.
- U11 143d60da0: input type query, returns byte (7d0198 path via 1407cf62d). Body beyond its first call to 143d5a140 (3d60daf-3d60db2) UNKNOWN.
- U12 143d5a140: sret accessor. Verified reads: `[rcx+0xe8]` and `[rcx+0xf0]` with control count check (3d5a14a-3d5a166). The full body and whether it is UWidget::GetCachedWidget are UNKNOWN.
- U13 global 0x8271318 (VA 0x148271318), passed as first arg to 14300d060. Meaning UNKNOWN.
- U14 global byte 0x80d1df0 (VA 0x1480d1df0): owning log category identity UNKNOWN. Read as verbosity.
- U15 global 0x80d1df4: log category object layout UNKNOWN.
- U16 global byte 0x7c522f0 (VA 0x147c522f0): flag passed as r9b to 14300d060. Meaning UNKNOWN.
- U17 path element type: stride 0x48, widget pointer at +0x38, inline capacity 4. Type name UNKNOWN.
- U18 Slate attribute internals at widget+0x190/+0x198/+0x1a0 and +0x1a8/+0x1b0/+0x1b8. The vtable slot 0x28 (bool) and slot 0x48 (byte, optional out-pointer) semantics are UNKNOWN. The dword vs qword test width differs between 3d5cb90 and 7d03bb; both are recorded as observed.
- U19 meaning of bit 0x10 in the byte cache at widget+0x1a8 (7d044e). Name not confirmed. The log text says AreChildrenHitTestVisible.
- U20 relation of rsp+0x68 (sret+8 slot, read at 7d02e5) to the pointer returned by 143d5a140 (7d02c7-7d02d2). The C++ treats them as the same buffer; this is UNVERIFIED.
- U21 caller struct: the record passed as rdx = this+0x20 by 140802440 (80262f). Its type and the meaning of this-0x28 are UNKNOWN.
- U22 helper 1407cf620 identity as a CommonButton-specific check. The body does not reference CommonButton. Name is address-suffixed (NativeHelper1407cf620); the CommonButton attribution comes from the task statement only.

## Not done
- No build, no run, no commit, no edits outside this folder.
- `policy.py` is not updated. Callee bodies U1-U12 are not translated, so the fragment cannot be enabled.
