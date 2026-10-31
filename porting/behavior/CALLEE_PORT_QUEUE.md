# Callee port queue (unidentified helpers that must be ported as functions)

Written by the second callee-identification pass (see `porting/decompilation_notes.md`, "Second pass 2").
Each entry is a callee from `BLOCKING_CALLEES.md` (rank and blocked count as there) that could not be written as an
engine helper or a one-line accessor. The decision says what to do with it. Addresses are from the slim project
`FortniteClient-Win64-Shipping.runtime.exe`. Decompile and disassembly: `porting/generated/callee_probe/` (git-ignored).
Third pass: rank 1 identified and its callers ported where the rest of the body is clear. The other rows are unchanged.
Fourth pass (see `porting/decompilation_notes.md`, "Fourth pass"): rank 4 `0x1419d3ff0` identified (accessor: `GetWorld()` at vtable `+0x150`, then `UWorld::GameState` at `+0x130`, kept if `IsA AFortGameStateAthena`). Row 7 `0x1406aaec0` stays unnamed (low-medium). `FUN_142c0c240` = `FText::FText()` and `FUN_14079d060` = `FText::operator=`, both high; they block only the `ClientBroadcastOn*` bodies, which the generator cannot yet reference (eight missing `InlineMulticastDelegate` members).

Decisions: **port-fn** = port the callee itself as a C++ function (game logic); **accessor** = a small body that
becomes C++ once one field or virtual slot is named; **struct** = a C++ special member (copy, move, destructor) of a
game struct, written once the struct type is known; **verify** = probably an engine function, check against UE 4.25
source before naming.

| Rank | Address | Blocked | Decision | What it is (evidence) | Still unidentified callees |
|---:|---|---:|---|---|---|
| 1 | `0x1406a9dc0` | 18 (13 alone) | identified (third pass): engine helper, not a port | IDENTIFIED: `TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>` (UE 4.25 ScriptDelegates.h, high). The stop rule above is wrong: the loop continues past skipped entries. The 14 delegate broadcasts that use it are ported (see `decompilation_notes.md`, third pass). | Dispatch over a TArray of 16-byte entries `{FWeakObjectPtr Target @+0, FName Function @+8}`. For each entry: resolve the target (`FWeakObjectPtr_Get`), stop at the first entry whose name is None or whose function is missing (`UObject_FindFunction`), call `Target->ProcessEvent(FindFunctionChecked(name), param)` through vtable `+0x210`, then remove the processed entries (`TArray16_ShrinkIfSparse`, memmove). 677-byte body. The stop rule is medium confidence. Needs: the 16-byte entry type, the UObject vtable index of `ProcessEvent` (`0x210 / 8 = 66`, confirm), and the callers `FUN_1406dcbb0`, `FUN_1406b1a90`. | none (direct callees are identified; `FUN_145803c70` is the stack cookie check) |
| 2 | `0x141a3c2b0` | 7 | port-fn | About 110 instructions. Walks an array whose count is the int at `+0x4f0`, calls `0x1406a9dc0` (rank 1) and `FUN_14070fe00` (float), and calls vtable `+0x770`. Depends on rank 1. | `FUN_14070fe00`, virtual `+0x770` |
| 3 | `0x141a3c390` | 7 | port-fn | Sums ten int32 counters at `+0xebc .. +0xefc` (stride 8) plus the qword at index `0x1d1` (`+0xe88`), compares with the qword at index `0x1e0` (`+0xf00`) and with the byte at `+0xf0`, calls vtable `+0x770` and `FUN_142af1340`. A threshold check on a game object. Needs the counter names. | virtual `+0x770`, `FUN_142af1340`, `FUN_1407ac640` |
| 4 | `0x1419d3ff0` | 7 | accessor | IDENTIFIED (fourth pass, high): accessor. `GetWorld()` (UObject vtable slot 42, `+0x150`), read `UWorld::GameState` at `+0x130`, return it if `IsA AFortGameStateAthena` (`UStruct::IsChildOfUsingStructArray`). Vtable order checked against the 4.25 headers (ProcessEvent = `+0x210`). Its 17 callers also need rank 2, 3 and 5. | none |
| 5 | `0x141991bf0` | 7 | accessor | 20 bytes: returns the pointer at `+0x1df8` if non-null, else `+0x1df0`. The same pair is tested by `0x141991ce0`, `0x14199e430`, `0x14199e4e0`, `0x1419920d0` and `0x141995550`. In `struct_layouts.json` the pair lies inside the `CurrentPlaylistInfo` struct of `AFortGameStateAthena` (`0x1cd0`, size 489), so the owner and the field name still need the reflection data. | none (field name only) |
| 6 | `0x1406ae340` | 3 | struct | Destructor: frees four heap buffers at `+0x30`, `+0x40`, `+0x50` and `+0x60` (`FMemory_Free`), then walks two more members. Write as the destructor of the game struct. | none (struct type) |
| 7 | `0x1406aaec0` | 3 | struct | Copy constructor of the same struct family: copies the fields, zeroes four members, increments the refcount of the object at `+0x30` (plain `+1`, not locked), then calls `0x143e8a690`. | `0x143e8a690` (row 9) |
| 8 | `0x142c64fe0` | 3 | struct | Move assignment: frees the target's buffer at `+0x0`, takes the source's pointer and count (`+0x0`, `+0x8`, `+0xc`) and zeroes the source. Also moves a second member at `+0x10`. | none (struct type) |
| 9 | `0x143e8a690` | 3 | struct | Copy assignment of a TArray: resizes through `FUN_1406d9a30` when the count differs, then `memcpy`. Element size not checked yet. A generic `TArray_CopyAssign` once the element type is named. 1252 call references. | `FUN_1406d9a30` |
| 10 | `0x143e8a780` | 2 | struct | Copy assignment of a struct with an owned buffer at `+0x10` (size not decoded): frees the old buffer (`FMemory_Free`), then copies the pointer and count pairs. Same family as rows 6 to 9. | none |
| 11 | `0x140f95ae0` | 2 | accessor | Calls vtable `+0x150` on the argument. If the pointer at `+0x188` of the result is null, returns the qword at `+0x320`; otherwise returns the qword at `+0xc0` of that pointer. Same shape as row 4. | virtual `+0x150` |
| 12 | `0x1420ee770` | 2 | accessor | Copies a 3-float vector from the argument into two members (`+0x22c` and `+0x1e2c`), then sets bit `0x8` of byte `+0x1e38` and bit `0x40` of byte `+0x38b`. A setter of 57 bytes, portable once the members are named. | field names |
| 13 | `0x1419ae980` | 2 | port-fn | Location and state colour helper (256 bytes). Calls `FUN_14468fb70`, `FUN_142a3d9f0`, `FUN_141ed6bb0`, `FUN_141edc1f0` and vtable `+0xaf0` and `+0x150`. Called by `AFortGameStateAthena::SetLocationColorInState`. | `FUN_14468fb70`, `FUN_142a3d9f0`, `FUN_141ed6bb0`, `FUN_141edc1f0`, virtual `+0xaf0` |
| 14 | `0x1419c3880` | 2 | port-fn | Probably `CanPlayerCreateInCurrentVolume` (guess from the first pass; not confirmed). 306 bytes, four vtable calls, calls `FUN_141c74820`, `FUN_141c7d420`, `FUN_14297b320`. Called from `ServerEndMinigame` and `ServerRestartMinigame`. | `FUN_141c74820`, `FUN_141c7d420`, `FUN_14297b320`, `FUN_14237f3d0`, `FUN_141eeab20` |
| 15 | `0x1440c99f0` | 2 | port-fn | Float computation: clamps two float inputs against zero and against each other, then calls `FUN_1440b73d0` and `FUN_14443cb90`, with four vtable calls (`+0x468`, `+0x498`, `+0x4c8`, `+0x860`). Needs the slot names. | `FUN_1440b73d0`, `FUN_14443cb90`, virtual slots |
| 16 | `0x144621fb0` | 2 | verify | Handle lookup: index in the low 24 bits of a 64-bit handle, element stride `0x90`, sparse index read from `+0x28` when set, otherwise the inline array at `+0x18`. Probably `FTimerManager::FindTimer`. Verify in `Engine/Private/TimerManager.cpp` before naming. | none |
| 17 | `0x141952190` | 1 (878 bytes) | port-fn | AFortGameModeAthena aircraft drop-zone handling. Strings: `FailedToLoad`, `NetworkErrors`, `Failed to load before game started.`. Nine indirect calls. Port as a whole function after the callees below. | `FUN_141991bf0` (row 5), `FUN_14468fb70`, `FUN_141990ec0`, `FUN_141695900`, `FUN_140fc7960`, `FUN_141971aa0`, `FUN_141971930` |

## Not queued

- `0x1406801c0`, `0x1406801d0`, `0x1406801e0` (identified as `Stub_Empty`, `Stub_ReturnTrue`, `Stub_ReturnFalse`):
  folded identical bodies, no port needed (empty, returns true, returns false).
- `0x142ec7720`, `0x142ec7730`, `0x142ee5dd0`, `0x142ee5e30`, `0x142ee7880`, `0x142d698b0`, the `*_StaticClass` getters and the
  other entries of `helpers.json`: identified, see `helpers.json`.
- `0x14199e430` and `0x14199e4e0` (playlist-info family): they use the pair accessor of row 5 and two checks
  (`FUN_141a49590`, `FUN_141a4b0d0`) that were not examined. Decide them after row 5.

## Reproduce

`porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input <list.json> --out porting/generated/callee_probe/<dir>`
then `porting/.venv/bin/python -I porting/ghidra/classify_callees.py --probe <dir> --out <dir>`. The probe rolls back its
transaction, so the project is not saved.
