# Legacy input dispatch: verified facts, divergences, UNKNOWN list

Image: /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (VA - 0x140000000).
Decompiles (git-ignored, porting/generated/decomp-imports/Imports): Funnel__1407c8d40, GlobalDispatchA/B/C,
GlobalSlot6, PanelDispatchA/B/C, PanelSlot6, Helper1407cc3b0__1407cc3b0 (new this pass).
Tags in definitions.inc: [asm] = checked against objdump; [dec] = Ghidra structure only, not asm-checked.

## Verified by asm (objdump windows)

- Global slot 0x10 1407c8be0 calls funnel 1407c8d40 at 7c8cbf. Slot 0x18 1407cab10 calls funnel at 7cabd3.
- Global slot 0x10 call args (7c8c9a-7c8cbf): rcx=Self, rdx=&Event (local 0x38), r8d=Arg4 (Priority),
  r9=Arg5 (Param4), [rsp+0x20]=&Delegate (local 0x50: word0 = 0x1407ad820, word2 = 0).
- Global slot 0x10 calls FUN_142ee3b30 with rdx=Self (7c8c1c). Guard init FUN_1407ae9d0 (7c8c80).
- Global slot 0x20 1407cc2b0 does NOT call the funnel. Calls 1407cc3b0 at 7cc35c with rcx=Self,
  edx=Arg2, r8d=Arg3, r9=Arg4, [rsp+0x20]=Arg5 (the 5th entry parameter, entry [rsp+0x28]),
  [rsp+0x28]=&Delegate (local 0x40: word0 = 0x1407ad780, word2 = 0). Calls FUN_142ee3b30 with rdx=Self
  (7cc2ec), guard FUN_1407ae9a0 (7cc33d). HANDOFF.md line 116 ("three input dispatch slots funnel through
  1407c8d40") is wrong for this slot.
- Global slot 0x8 140695430: `lea rax,[rcx-0x28]; ret` (net -0x28).
- Global slot 0x0 1407a5da8: `sub rcx,0x28; jmp 0x7a7d10`. Dtor not translated.
- Panel slot 0x0 1407a5d84: `sub rcx,0x2d8; jmp 0x7a78d0`. Not translated.
- Panel slot 0x8 1408056ac: `sub rcx,0x1d0; jmp 0x6a7c40`. Net -0x2d8 (0x6a7c40 returns rcx-0x108).
- Panel slots 0x10 1407c87e0, 0x18 1407ca7f0, 0x20 1407cbf10 begin with stack spills (no thunk adjustor).
- Helper 1407cc3b0 is called from slot 0x20 only (asm 7cc35c). Decompile call list: uses eligibility 1407d0160,
  FUN_142ee3b30, FUN_142ee5dd0; does NOT use filter 1407cf950 or FUN_14079a3a0 (no Event release on success).

### Funnel 1407c8d40 branch check (objdump 7c8d40..7c930e, all branches compared)

- Entry: FUN_142ee3b30(temp, 0) with temp in a local (7c8d9c). Loop test cmp [this+0x58] (7c8df1).
- Record = [this+0x50] + i*0x90 (7c8dfb). Row = FUN_14078c640(record, msg) (7c8e15).
- Skip (advance) when record+0x18 < this+0x68 unsigned (7c8e1a-7c8e26, jb 7c8f5f) or row null (7c8e32).
- Filter FUN_1407cf950(row, Event, FUN_142ee5dd0(this+0x60)) false: advance (7c8e4a-7c8e51, je 7c8f5f).
- Target: FUN_142ee7880(record+0x10) false: bValid=0 (7c8e5e-7c8e65). True: bit0 set, FUN_142ee5dd0(record+0x10),
  FUN_143d5a140(ret, out) (7c8e6e-7c8e7a), bValid = [ret] != 0 (cmpq $0,(%rax), 7c8e7f). Out object released
  after this when bit0 set (7c8e8d-7c8ec2).
- bValid false: FUN_1407f0790(this+0x50, i, 1, 1), index not advanced (7c8f3d-7c8f4c, 7c8f58-7c8f64).
- Eligibility FUN_1407d0160(this, record+0x10) false: advance (7c8ecc-7c8ede, je 7c8f36/7c8f64).
- Key = (record & 0xffffffff00000000) | Priority: 7c8e10 mov %rbx,0x38(rsp), 7c8ee0 mov %eax,0x38(rsp).
- Target object: word2 if nonzero else [Delegate+0x20] (7c8ef6-7c8f02); this = vtable[+8](target) (7c8f09).
- Invoke: Delegate->word0(this, record, &RowData slot, &Key, Param4 [rsp+0x20], &bOut [rsp+0x28]) (7c8f0c-7c8f2f).
- Invoke false: advance (7c8f34 -> 7c8f36 -> 7c8f64).
- Invoke true (success): bOut returned (7c8fae movzbl), FUN_14079a3a0(Event) (7c9122), then
  if word0 != 0: vtable[+0x10] on target (7c9127-7c913e). Event word2 NOT released here (7c9145 -> 7c92fb).
- Loop exhausted (fall-through): cleanup, Event word2 released (7c92a4-7c92c8), then if word0 != 0:
  vtable[+0x10] (7c92de-7c92f6), return false (7c92f9 xor al).
- Cleanup blocks (arrays, FUN_142bfd200 / FUN_142bee710): decompile only, not asm-checked in full.

## Divergences from the earlier decompile-based translation (all fixed in definitions.inc)

D1  First helper call is FUN_142ee3b30(temp, 0), not (this). [asm 7c8d9c]
D2  Filter false and record+0x18 skip both ADVANCE. Earlier code removed the record for filter false. [asm 7c8e1a-7c8e51]
D3  Target test: out-struct is a parameter of FUN_143d5a140; validity = first qword of its return != 0
    (not "return != nullptr"); out object released. [asm 7c8e6e-7c8e8d]
D4  Eligibility is called with (this, record+0x10), not the row. [asm 7c8ecf-7c8ed2]; declaration corrected.
D5  Key high dword comes from the record ADDRESS, not Record->Key. [asm 7c8e10, 7c8ee0]
D6  Delegate invoke: 3rd arg is the address of a RowData slot (not the row); this-adjust via vtable[+8] first. [asm 7c8ef6-7c8f2f]
D7  Fall-through path calls Delegate finish (vtable+0x10) when word0 != 0. Earlier code omitted it. [asm 7c92de]
D8  Global slot 0x10 funnel call: 4 args + Delegate on stack (earlier code had 3 args). [asm 7c8ca7-7c8cbf]
D9  Global slot 0x20 signature: 5 args + Delegate for helper 1407cc3b0 (earlier declaration had 4). [asm 7cc35c]
D10 Global slot 0x20 FUN_142ee3b30 call uses rdx=Self (earlier wrapper matched; now explicit). [asm 7cc2ec]
D11 Global slot 0x18 funnel call: Delegate argument not asm-checked; now an UNKNOWN placeholder (U21), not silently dropped.
D12 Refcount observation (old U22): asm shows the funnel releases Event word2 only on fall-through, never on
    the success path. Whether FUN_14079a3a0 consumes the copy on success is U11.

## UNKNOWN list (U1-U23; this pass supersedes the old numbering)

U1  FUN_142ee3b30 temp init semantics. Calls: funnel (temp,0) 7c8d9c; slot 0x10 (temp,Self) 7c8c1c; slot 0x20 (temp,Self) 7cc2ec.
U2  FUN_14078c640 row lookup (message "CommonUIUtils::GetInputActionData couldn't find the row").
U3  FUN_142ee5dd0 and the meaning of this+0x60 (NativeField60).
U4  FUN_1407cf950 filter(row, Event, U3 result).
U5  FUN_142ee7880 validity test.
U6  FUN_143d5a140 object resolve; out-struct layout (object pointer at out+0x8 per asm 7c8e91-7c8ebf).
U7  FUN_1407f0790 record removal beyond its args.
U8  Delegate layout: word0 invoke convention beyond the asm-verified args; word1 meaning; object type behind word2.
U9  Delegate finish effect (vtable+0x10).
U10 Cleanup helpers FUN_142bfd200, FUN_142bee710, vtable+0x38 (cleanup order and effect).
U11 FUN_14079a3a0 effect on Event (called on success; whether it consumes the Event copy).
U12 Object type behind Event word2 (vtable[0], vtable[1] destroy and delete).
U13 Types of this+0x50, +0x58, +0x68 (record array, count, threshold).
U14 Guards FUN_1407ae9d0 (slots 0x10, vtable 1459feea8), FUN_1407ae9a0 (slots 0x18/0x20, vtable 1459feee8),
    FUN_1407ae640 (panel owner guard), guard finish (vtable+0x10 on guard or its inner object).
U15 FCommonInputEventNative, FCommonDispatchPairNative, FCommonDelegateNative layouts (word0/word1 meanings).
U16 Dtor bodies 0x7a7d10 (global 0x0) and 0x7a78d0 (panel 0x0).
U17 FUN_14080e600 (global 0x30 registration index) and FUN_1407cac50 (global/panel 0x30 state call), FUN_140811c70.
U18 Panel/row helpers: FUN_14081b480, FUN_14081b530 (mode index), FUN_14081b540 (mode tag), FUN_1407c5190
    (args: record+0x60, tag, index), FUN_1407bc6e0, FUN_140687d00 (set-bit iterator; start index layout),
    owner vtable+0x290.
U19 Panel helpers: FUN_1407cf0f0, FUN_1407c20f0 (row state float at +0x1c, flag at +0x19), FUN_142ec9900,
    FUN_140806b60, FUN_1407f17d0.
U20 Delegate bodies at 0x1407ad820 (global 0x10 delegate word0) and 0x1407ad780 (global 0x20).
U21 Global slot 0x18 funnel Delegate argument (5th, stack): not asm-checked, object unknown.
U22 Helper 1407cc3b0 Key high dword: decompile never writes auStack_128[1]; value UNKNOWN.
U23 Panel slot 0x10 Arg4==1 path: decompile calls vtable slot +0x30 on InterfaceThis; translation calls
    140800860 directly. Vtable slot identity not verified.

Out of scope per task: tick bodies 140802440 and 140802200. External: eligibility 1407d0160 / 1407cf620 (another worker).

## Not asm-checked (translated from Ghidra structure only)

- Helper 1407cc3b0 (all branches), global slot 0x30 140800b70, panel slots 0x10, 0x18, 0x20, 0x30.
- Global slot 0x18 call args (D11). Panel 0x20 nesting of the Cls==1 path under the Arg3==0 check (inferred).
- Global slot 0x10 guard finish selection (decompile picks guard inner object or guard self).
- Funnel cleanup blocks (decompile only).

## Status

- UNKNOWN count: 23 (U1-U23).
- definitions.inc: funnel [asm] fixed per D1-D7; global 0x10 and 0x20 call sites [asm]; others [dec].
- Only files in this folder were written; the Ghidra runs wrote decompiles to the git-ignored generated folder.
- No build, no run, no commit.
