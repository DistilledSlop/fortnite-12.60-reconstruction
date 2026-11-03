# HideToast translation: resolution round 4 (delegates)

Evidence: image-125.runtime.bin (file offset = VA - 0x140000000), objdump -D -b binary -mi386:x86-64.
Engine: Core/Public/UObject/ScriptDelegates.h. Reflection: fortnite_game_reflection.json. Nothing here is a guess.
Anything not listed as VERIFIED stays UNKNOWN. No build, no run, no commit.

## VERIFIED

### Container layout (stride conflict resolved)
- Add site (0x144e22b70, +0x78 path): `lea 0x78(%rax),%r15` at 0x144e22be2, where rax = Get(this+0x290) (weak pointer, 0x144e22bc8-0x144e22bd8).
  Entry write: `shl $0x4,%rax; add (%r15),%rax` at 0x144e22c77-0x144e22c7b; then `mov %rbx,(%rax)` (0x144e22c8c) and
  `mov %edi,0x8(%rax)` (0x144e22c97). Num: `mov %eax,0x8(%r15)` (0x144e22c60), Max compared at 0xc(%r15) (0x144e22c64).
  => element stride is 16 bytes: qword pointer to a heap object, then int32 tag (edi = 3). Data at +0, Num at +8, Max at +0xc.
- RemoveAll loop 0x140690620: `shl $0x4,%rbp` (0x140690652), entries at (%rbx), `cmp %esi,0x8(%rbx)` (0x140690660), so 16-byte stride.
- The 0x30 size belongs to the heap object the entry points to (allocated via 0x142bfd200, LISTENERS.md), not to the container stride.
  The 0x30 "stride" is therefore not a conflict. (The allocation size argument is in LISTENERS.md, not re-verified here.)
- Broadcast guard for Player +0x250 (site 0x1862b8d): `cmpl $0x0,0x258(%rbx)` (0x1862b73) = Num at container+0x8, consistent with Data at +0x250.
- Engine: FMulticastScriptDelegate InvocationList is `TArray<TScriptDelegate<TWeakPtr>>` (ScriptDelegates.h:613-618); TScriptDelegate is
  TWeakPtr Object (8 bytes) + FName FunctionName (8 bytes) (ScriptDelegates.h:253-256), i.e. 16 bytes.

### Items NOT resolved by this window
- The RemoveAll early exit reads `cmp %esi,0x14(%rcx); jle` (asm 0x140690640-0x140690646 region): it returns when int32 at container+0x14 <= 0.
  The Add site never writes container+0x10 or +0x14. Meaning of +0x14 is UNKNOWN; this is the one remaining container conflict.
- The entry's first qword is a raw pointer to an object with a vtable (called at +0x18 and +0x38 in 0x140690620, asm 0x140690673 and 0x14069068c; at +0x30 in the Add site, asm 0x144e22c56), not a
  TWeakPtr. So the entry is NOT verified to be the engine FScriptDelegate layout. Dynamic-multicast identity: UNKNOWN.

### Broadcast sites (find_field_refs, window 0x200000 around 0x144e0c9c0, output /tmp/claude-1000/new-r2/refs.json)
- ProcessMulticastDelegate 0x1406a9dc0 has 1825 call sites in image-125 (objdump search). Sites that load rcx with
  `lea 0x78|0x250|0xe8(reg),%rcx` immediately before the call: 33 (scan output in /tmp/claude-1000/new-r2/scripts/bc.py run).
- Site 0x1862b8d (Player +0x250, receiver rbx): the only store to the parameter block is `mov %rbx,0x30(%rsp)` (0x1862b83), then rdx = 0x30(%rsp).
  So one 8-byte parameter slot is visible; the block size and the parameter type are NOT verified. The 2-parameter callback 0x144e00850
  (Ghidra count) does not match one visible slot; not resolved.
- Site 0x1d8be91 (profile +0xe8 receiver rsi): one 8-byte slot `mov %rbx,0x30(%rsp)` (0x1d8be87); rbx source not traced; owner UNKNOWN.

## UNKNOWN (count: 10)
1. +0x78 delegate kind and signature (no verified broadcast site for +0x78 in the HideToast path).
2. +0x78 owner: UFortTeamMember per RESOLVED.md item 2 is verified only for storing zero; element class of heap object (vtables 0x1459a9e80/0x1459a9ed8) UNKNOWN.
3. Player +0x250: owner class (no reflected property, no verified constructor store).
4. Player +0x250: heap element class (vtables 0x1459cb3b8/0x1459cb410 not bound to a name).
5. Player +0x250: signature (one 8-byte slot visible at 0x1862b83; block size and types unverified; callback 0x144e00850 is 2 params per Ghidra).
6. Profile +0xe8: owner class (not reflected; no verified constructor store).
7. Profile +0xe8: heap element class and signature (callback 0x144e07780, 4 params per Ghidra, types unverified).
8. Meaning of container+0x14 in the RemoveAll early exit (0x140690640).
9. Whether the containers are engine FMulticastScriptDelegate (entry layout not verified).
10. Identity of the Broadcast receiver class at 0x1862b8d and 0x1d8be91 (containing functions not decompiled; no class binding).
