# Client-bot flags +0x1818 / +0x1838: writer scan (round 2)

Status: partial. Not built, not run, not committed. Scratch: `/tmp/claude-1000/-opt-fortnite/af88edb0-41ba-493f-b44b-98edf8730630/scratchpad/botflags2/`.
Tool: `porting/ghidra/find_field_refs.py --window 0x400000 --all-access`. Decompiles: `porting/generated/decomp-imports/Imports/`.

## Verified

- Owner class: reflection group 2708 (`porting/generated/reflection/fortnite_game_reflection.json`, `classes_in_groups_with_fort_names`) has `BotManager` at offset 6192 (0x1830, table index 111) and `ClientBotManagerClass` at 6208 (0x1840, index 110). Both match the header `porting/native/Source/FortniteGame/Generated/AFortPlayerController.h` lines 802 and 806. The group name is unresolved in the JSON; the match to AFortPlayerController is by these indices and offsets.
- Reflected properties at 0x1818 and 0x1838 in group 2708: none. Offset 6168 (0x1818) is `Hero` in group 2733 and `EnergyAmmoItemDefinition` in group 2129. Offset 6200 (0x1838) is `FortInputData` in group 2129. Neither belongs to group 2708.
- Byte stores to +0x1818 (scan anchored at 0x1405b3b76 and 0x142378d49):
  - `0x1405b3b76`: `MOV byte ptr [RBP+0x1818],0xff`. Function `BotA_1405b13f0` (0x1405b13f0). Decompile is unusable (2549 lines, offset not rendered).
  - `0x1405b3b9f`: `MOV byte ptr [RBP+0x1818],0x2`. Same function. The value 2 shows the byte is not a pure bool.
  - `0x142378d49`: `MOV byte ptr [RBP+0x1818],0x1`. Function `BotB_142378c00` (0x142378c00). Decompile line 59: `*(undefined1 *)(param_1 + 0x1818) = 1;`, after a `FrontEnd` string path (`FUN_142bc7780(..., L"FrontEnd", 0x12)`) and `FUN_141ce8b80`, followed by `FUN_142a3f8b0(param_1,1)`. Whether `param_1` is the controller is UNKNOWN.
- Byte store to +0x1838: none found. Byte accesses to 0x1838 are reads only: `CMP byte ptr [RCX+0x1838],0` at `0x142370a98` (function `BotE_142370a90`, decompile line 27 tests 0x1838 || 0x1818).
- Qword stores that cover the byte (class not verified): `0x141faf167` (`MOV qword ptr [RBP+0x1818],RAX`, function 0x141fa6320); `0x142325d5a` (`MOV qword ptr [RDI+0x1818],RBX`, function 0x142325d00).
- Qword store to +0x1838: `0x1422b652e` (`MOV qword ptr [R14+0x1838],RBP`, function `BotD_1422b6090`). Pointer-width, not a bool write.
- Address-of (LEA) to +0x1838: `0x1426f0b02` and `0x1426f0b41` in function 0x1426eb630 (`BotC_1426eb630`, 5189-line decompile with no rendered offset). Whether the pointer is passed to a byte writer is UNKNOWN.
- Earlier round: `0x1411e0307` `andb $0xfc,0x1818(%rbx)` remains the only AND-byte site found.

## Not found / UNKNOWN

- No `OR byte ptr`, `AND byte ptr`, `BT`, or `TEST` writer to 0x1818 or 0x1838 was seen in the anchored scans, other than the `andb` site above.
- No constructor default for either byte was identified. `BotF_1408bcdb0` (LEA [RBP+0x1818] at 0x1408c02bd) failed to decompile twice (timeout, then failed).
- Triggers for `BotA_1405b13f0`, `BotB_142378c00`, `BotC_1426eb630`, `BotD_1422b6090` are UNKNOWN beyond the instruction facts above. `FUN_142a3f8b0`, `FUN_144a61520`, `FUN_141ce8b80` are unnamed.
- Many `CMP qword ptr [reg+0x1818],0` readers exist (for example 0x142319f7c, 0x142345f83). These are reads only.
- Stack-relative (`[RSP+0x1818]`/`[RSP+0x1838]`) hits are excluded as not relevant.

## Tool note

Each `decompile_targets.py` run reported `project_files_changed: 5` for files under `/opt/fortnite/ghidra-projects/fortnite-slim.rep/`. The tool docstring says the program is not saved. This was observed and not investigated further.
