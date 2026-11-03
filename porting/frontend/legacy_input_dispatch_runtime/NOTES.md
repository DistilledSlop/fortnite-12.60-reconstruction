# Legacy input dispatch: verified facts and UNKNOWN list

Image: /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (VA - 0x140000000).
Decompile: porting/generated/decomp-imports/Imports/Funnel__1407c8d40.c (new, Ghidra slim; decompile only,
git-ignored). Other slot decompiles already in that folder: GlobalDispatchA/B/C, GlobalSlot6, PanelDispatchA/B/C, PanelSlot6.

## Verified by asm (objdump windows)

- Global slot 0x10 1407c8be0 calls funnel 1407c8d40 at 7c8cbf. Slot 0x18 1407cab10 calls funnel at 7cabd3.
- Global slot 0x20 1407cc2b0 does NOT call the funnel. It calls 1407cc3b0 at 7cc35c. HANDOFF.md
  line 116 ("three input dispatch slots funnel through 1407c8d40") is wrong for this slot.
- Global slot 0x8 140695430: `lea rax,[rcx-0x28]; ret` (net -0x28).
- Global slot 0x0 1407a5da8: `sub rcx,0x28; jmp 0x7a7d10`. 0x7a7d10 (dtor) not translated.
- Panel slot 0x0 1407a5d84: `sub rcx,0x2d8; jmp 0x7a78d0`. 0x7a78d0 not translated.
- Panel slot 0x8 1408056ac: `sub rcx,0x1d0; jmp 0x6a7c40`. Per prior NOTES.md pass, 0x6a7c40 returns rcx-0x108;
  net -0x2d8 (that tail was not re-read in this pass).
- Panel slots 0x10 1407c87e0, 0x18 1407ca7f0, 0x20 1407cbf10 begin with stack spills, not `sub rcx`
  (no thunk adjustor at entry).

## Structure from Ghidra decompile (branch structure NOT checked against asm, branch by branch)

- Funnel params: Event (3-word copy from slot; word 2 intrusive refcounted), Priority (int32), Param4 (uint64),
  Delegate (word 0 function pointer, word 2 object or inline at +0x20).
- Loop over records at this+0x50, count this+0x58, record size 0x90. Record+0x18 compared (unsigned) with
  this+0x68. Null row or failed compare: skip.
- Failed hold/target test (FUN_142ee7880 false or FUN_143d5a140 result null): remove record (FUN_1407f0790 with
  args (this+0x50, index, 1, 1)) and do NOT advance index.
- Eligibility 1407d0160 (external, not translated) false: advance index.
- Success (delegate returns true): out-bool from delegate result is returned after cleanup and optional finish.
- Fall-through: cleanup, then release Event word 2 (refcount dec; zero -> vtable[0] destroy, weak dec ->
  vtable[1](obj,1)).
- Observed (decompile only): on the success path the funnel does not release Event word 2; on the fall-through
  path it does. Each slot releases its own copy of word 2 after the call. Net refcount effect on the failure
  path therefore looks like -1 extra. NOT verified by asm; treat as unresolved until asm check.

## UNKNOWN list (each stays unresolved; asm address given)

U1  FUN_142ee3b30 temp init, funnel 1407c8d40 entry (+0x... decompile call site)
U2  FUN_14078c640 row lookup (message "CommonUIUtils::GetInputActionData couldn't find the row")
U3  FUN_142ee5dd0 applied to this+0x60; meaning of +0x60 field (NativeField60)
U4  FUN_1407cf950 filter(row, Event, U3 result)
U5  FUN_142ee7880 validity of record+0x10
U6  FUN_143d5a140 object resolve for record+0x10
U7  FUN_1407f0790 record removal (semantics beyond args)
U8  Delegate layout: word 0 invoke convention, word 2 object, inline offset (decompile +0x20)
U9  Delegate finish call (vtable +0x10) and its effect
U10 Release/held helpers in cleanup: FUN_142bfd200, FUN_142bee710, vtable +0x38 call
U11 FUN_14079a3a0 effect on Event
U12 Object type behind Event word 2 (vtable[0], vtable[1])
U13 Meaning of this+0x68 (int32) and this+0x50/+0x58 field types (NativeField68 etc.)
U14 Guards FUN_1407ae9d0 (slot 0x10, vtable 1459feea8) and FUN_1407ae9a0 (slots 0x18/0x20, vtable 1459feee8)
U15 FUN_142ee3b30(this-0x28) call in slots (same as U1 in the interface thunks)
U16 Body of FUN_1407cc3b0 (slot 0x20 helper)
U17 Slot 0x20 param Arg5: stored to a local, use UNKNOWN
U18 FCommonInputEventNative / FCommonDispatchPairNative layouts (words 0,1 meaning)
U19 Dtor bodies 0x7a7d10 (global 0x0) and 0x7a78d0 (panel 0x0)
U20 Global slot 0x30 140800b70 (state lookup), not translated
U21 Panel slots 0x10 1407c87e0, 0x18 1407ca7f0, 0x20 1407cbf10 and 0x30 140800860, not translated
U22 Net refcount observation on funnel failure path (see above), unverified

Out of scope per task: tick bodies 140802440 and 140802200. External: eligibility 1407d0160 / 1407cf620.

## Status

- UNKNOWN count: 22 (U1-U22).
- No build, no run, no commit. Only files in this folder were written, plus the generated decompile output above.
- definitions.inc is a structure translation of the funnel and the three global dispatch slots; it is not
  asm-verified branch by branch and helper bodies are not implemented.
