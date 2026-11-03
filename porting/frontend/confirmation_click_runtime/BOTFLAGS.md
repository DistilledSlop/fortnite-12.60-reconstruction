# Client-bot flags +0x1818 / +0x1838 on AFortPlayerController

Status: producers NOT recovered. Evidence is partial. Not built, not run, not committed.

## Verified

- Dump: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base 0x140000000.
  File offset = VA - 0x140000000. Checked by sha256 of window `node_shared_delegate` (0x1407899c0, 0x120 bytes) matching `common_action_route_connected_runtime/original-windows.json`.
- Reader, IsClientBot region (VA 0x142a47ff0 onward):
  - `cmpb $0x0, 0x1838(%rcx)` at 0x142a48005, then `cmpb $0x0, 0x1818(%rcx)` at 0x142a4800e.
  - Writes `movb $0/$1, (%r8)`. Result is `(+0x1838 || +0x1818) != 0`. Reads only, no writes.
- BotClickButton body (144f54030) also reads both bytes: cmp at 0x144f54077 and 0x144f5406e.
- Byte-width stores found by a whole-dump scan (disp32 0x1818 / 0x1838, opcode before modrm):
  - 0x1405b3b76: `movb $0xff, 0x1818(%rbp)`. Containing function not identified.
  - 0x142378d49: `movb $0x1, 0x1818(%rbp)`, after `call 0x1ce8b80`. Containing function not identified.
  - Neither scan hit is a byte store to 0x1838.
  - So only 0x1818 has an immediate byte store. Whether `rbp` is the controller pointer in either function is unverified.
- 0x1411e0307: `andb $0xfc, 0x1818(%rbx)`. Clears the low 2 bits of the byte at 0x1818. This suggests 0x1818 may be a packed flag byte, not a pure bool. It sits in a struct-init block (movss/movq at 0x17f0..0x1820).
- Other hits (0x1417209d5, 0x1415d1774, 0x140a752b7, 0x14393b6f8, 0x1ebd448, 0x14197fe81, 0x141315f22) are 4- or 8-byte stores inside wider struct-init blocks. They are not bool-specific writers.
- Generated: `porting/native/Source/FortniteGame/Generated/AFortPlayerController*.h` has no `bNativeClientBot*` members. Its ClientBot entries are `BotManager`, `ClientBotManagerClass`, and the `ClientBot*` UFUNCTIONs.
- `controller-members.inc` declares `bNativeClientBotFlag1818` and `bNativeClientBotFlag1838` as `bool = false` placeholders.

## Inferred (not verified)

- The two `movb` stores above are candidate producers for +0x1818. No producer for +0x1838 was found.
- The `andb $0xfc` site suggests the byte may be bitfield storage. The `bool` typing in the members file may be wrong.
- Generic 8-byte zeroing (`movq $0`) in ctor-like blocks would also clear these bytes. Those sites could be constructor defaults, but that is unverified.

## Limits of the scan

- Only checked disp32 encodings with mod=10 and the opcode at i-2 (no SIB). Stores addressed through SIB (e.g. `[reg+idx+0x1818]`) were missed.
- No enclosing functions were identified. Mapping the `movb` sites to their owning class and their call sites is the next step.
