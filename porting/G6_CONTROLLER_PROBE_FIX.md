# G6: why the controller memory probe found nothing, and how to fix it

Date: 2026-11-12. Status: **static analysis only, not yet run against a live client.**
Applies to `porting/tools/pc_probe.py` (untracked) and to the G6 notes in `MATCHMAKING_PROBE.md`.

## Summary

The probe looks for the client's player controller by its vtable pointer and found 0 objects in runs 6 to 9.
The memory read was fine. **Every vtable address the probe used was not the start of a table**, so no object's first
qword could ever equal it.

| Probe used | Real table start | Error |
|---|---|---|
| `0x14613f338`, `0x145e1aac8` | `0x14613f330`, `0x145e1aac0` | 8 bytes too high |
| `0x14613d0a8` | `0x14613d310` | 0x268 too low (inside earlier tables) |
| `0x1461a7fe0`, `0x1461aa4b0`, `0x1461ae140`, `0x1461ba588` | `0x1461a8248`, `0x1461aa718`, `0x1461ae3a8`, `0x1461ba7f0` | 0x268 too low |

The "any `.rdata` pointer whose slot holds a known function" test failed for the same reason: all three slot
numbers in it (`+0x1570`, `+0x1588`, `+0x1d00`) were measured from the wrong starts.

## Cause

This client has no RTTI, so vtables sit back to back in `.rdata` with nothing between them.

- `tools/vt.py` finds a table start by walking back while the previous qword points into `.text`. With tables back
  to back it walks through several neighbouring tables. For the quickbar function it stopped at `0x14613d0a8` and
  reported "slot `+0x1d00`". The real table starts at `0x14613d310` and the real slot is `+0x1a98`.
- That false `+0x1d00` happened to match a real `call [rax+0x1d00]` inside `OnReadyToStartMatch` (`0x1423dfe20`).
  It looked like confirmation, but that call goes to `0x14211ef70`, which does not touch `ClientQuickBars`.
- The second attempt (`0x14613f338`, slots `+0x1570`/`+0x1588`) was off by one slot.

## How a real table start is found

A primary vtable start is the target of a `lea reg,[rip+X]` in a constructor, followed by a store to `[this]`.
Collect every RIP-relative `lea` target in `.text` that lands in `.rdata`; from a function-pointer hit, walk back to
the nearest such target.

```python
# buf = image bytes as numpy uint8, text = buf[.text]; file offsets equal RVAs in the runtime dump
c = ((text[:-7] == 0x48) | (text[:-7] == 0x4c)) & (text[1:-6] == 0x8d) & ((text[2:-5] & 0xc7) == 0x05)
idx = np.nonzero(c)[0]                     # REX.W lea with modrm mod=00 rm=101
disp = text[idx+3:...] as little-endian int32
targets = TEXT_VA + idx + 7 + disp         # keep those in .rdata and 8-aligned
# table start for a hit = first address <= hit that is in `targets`, with only .text pointers in between
```

Check on the result: each start below is loaded by exactly one constructor, and the next instruction is
`mov [rbx],rax` (`48 89 03`) or `mov [rdi],rax`. Example: `0x1423c4be4: lea rax,[0x14613f330]; mov [rbx],rax`.

## Correct tables and slots

Sections: `.text` `0x140001000`-`0x1458fbc00`, `.rdata` `0x1458fc000`-`0x147811c00`. The image loads at its
preferred base under Wine (confirmed in run 9: the process holds the same bytes at the same addresses).

All 16 player-controller tables hold the per-tick driver `0x1423bc750` at slot `+0x1570`:

| Table start | Size | `+0x918` ClientRestart | `+0x1578` check | `+0x1590` OnReadyToStartMatch | `+0x1a98` make quickbars |
|---|---|---|---|---|---|
| `0x145e1aac0` | 0x2930 | `0x1419c76a0` | `0x1423cc220` | `0x1419e6130` | `0x1419fadc0` |
| `0x14613f330` | 0x2290 | `0x1423cde50` | `0x1423cc220` | `0x1423df310` | `0x1406801c0` (empty) |
| `0x1461a8248` | 0x22e8 | `0x1423cde50` | `0x1423cc220` | `0x14254fea0` | `0x1423f20a0` |
| `0x1461aa718` | 0x2348 | `0x1423cde50` | `0x1423cc220` | `0x14254fea0` | `0x1423f20a0` |
| `0x1461ae3a8` | 0x2368 | `0x1423cde50` | `0x1423cc220` | `0x14254fea0` | `0x1423f20a0` |
| `0x1461ba7f0` | 0x22e8 | `0x1423cde50` | `0x1423cc220` | `0x14254fea0` | `0x1423f20a0` |
| `0x14613d310` | 0x2020 | `0x142372de0` | `0x14236fc30` | `0x14239ccf0` | `0x1423f20a0` |
| `0x14613b108` | 0x2020 | `0x142372de0` | `0x14236fc30` | `0x14239ccf0` | `0x1406801c0` |
| `0x146128208`, `0x1461e7520` | 0x1e80 | `0x142372de0` | `0x14236fc30` | `0x14239ccf0` | `0x1406801c0` |
| `0x145e3d2f8`, `0x14612a270`, `0x14612c5e8`, `0x146141e40`, `0x146150078`, `0x14616a200` | 0x2178-0x21b8 | `0x142372de0` | `0x14236fc30` | `0x1423df270` | `0x1419fadc0` |

Class names are inferred, not proven. Constructor calls give the hierarchy: `0x14613f330` is the parent of
`0x145e1aac0` and of `0x1461a8248`, which is the parent of the other three `0x1461…` tables. `0x145e1aac0` is the
largest table and its overrides live in the Athena code range (`0x1419…`), so it is most likely
`AFortPlayerControllerAthena`, the class the match session uses. The lobby controller is one of the tables in the
lower half. The probe does not need the names (see the fix).

## What the client actually does each tick

Driver `0x1423bc750` (slot `+0x1570`, same function in all 16 tables), from the disassembly:

```
obj = [this+0x220]
if obj == null or obj is not an instance of the class from 0x142a85850: return     // gate 1
if [this+0x658] & 0x20: return                                                     // gate 2
if not vcall +0x14e8: vcall +0x1578        // +0x14e8 = 0x14238b050: [this+0x6f8] && [this+0x6f9]
if vcall +0x14e8:                          // +0x1578 = the check, 0x1423cc220
    if [this+0xf0] != 3: call 0x142a3f150
    vcall +0x1590                          // OnReadyToStartMatch
```

Corrections to the earlier notes:

- `ClientQuickBars` (+0x14d0) is created through slot **`+0x1a98`**, called at `0x14239d167` inside the base
  `OnReadyToStartMatch` (`0x14239ccf0`; `0x14239d158` is a `.pdata` chunk of it). The Athena override
  `0x1419e6130` calls `0x1423df310`, which calls `0x14239ccf0`. The call is skipped when `0x143f7a920` returns 1 or
  the field is already set.
- `0x1423f20a0` and `0x1419fadc0` are both real creators (same guard, they store to +0x14d0). They are identical
  bodies shared by unrelated tables, so they do not identify a class.
- Two gates sit **before** the check `0x1423cc220` runs at all. If gate 1 or gate 2 fails, none of the check's
  log lines can print. That fits run 4 to 9, where `LogFort VeryVerbose` printed none of them. The earlier notes
  only considered gates inside the check.

`0x142a85850` is a class getter in the engine code range (cached pointer at `0x1481b6550`). It is probably
`ULocalPlayer::StaticClass` and `+0x220` the controller's `Player`, but neither is confirmed.

## Fix for `pc_probe.py`

1. Replace the vtable test. An address `v` is a controller table when `v` is in `.rdata` and
   `[v+0x1570] == 0x1423bc750`. This is one test for all 16 classes and needs no class list. Remove `SLOT`,
   `SLOT_FN` and the `+0x1570`/`+0x1588`/`+0x1d00` checks.
2. Print the driver's inputs for each hit, in gate order: `[+0x220]` (and whether it is null), byte `[+0x658]`
   bit 0x20, `[+0x6f8]`, `[+0x6f9]`, `[+0xf0]`, then the existing `+0x14d0`, `+0x1738`, `+0x19e8` fields. Print the
   table address so the class is visible.
3. Stop hiding read failures. A region whose read raises is skipped silently today. Read in chunks of 64 MB or
   less and print the number of bytes that could not be read. Drop the "skip regions over 1 GB" rule or report
   what it skipped.
4. Add a self-test that needs no client: load the runtime image, confirm each table start is a `lea` target and
   that `[start+0x1570] == 0x1423bc750`. Fail loudly otherwise.
5. Replace `vt.py`'s walk-back with the `lea`-anchored start, or print a warning that its "vtable start" and
   "slot" are unreliable in this image.

## How to validate without wasting a run

The lobby has a player controller too, so the probe can be proven before the match starts, inside one run:

1. Run the self-test (step 4). No client needed.
2. Start one `tools/vanilla_play_run.sh` run. When the client is in the lobby, run the probe. **It must print at
   least one controller.** If it prints none, stop the run; the probe is still wrong and the match scan is
   worthless.
3. Only if step 2 passes, let the run reach the match and scan about 60 s after `Athena pawn spawned`.

## Reading the match result

| Probe shows | Meaning | Next step |
|---|---|---|
| `[+0x220]` is null | Gate 1: the match controller has no local player object | Look at how the client assigns it after travel; check for a second, unowned controller |
| `[+0x658] & 0x20` set | Gate 2 | Find what sets and clears that bit (search for `or byte [reg+0x658],0x20`) |
| `+0x6f9` = 1, `+0x6f8` = 0, gates 1 and 2 pass | The check `0x1423cc220` runs and fails inside | Use the existing notes: foundations (`0x14236a740`), loading counter (`0x143a9f860`), virtual `+0x2170` |
| `+0x6f8` = 1 and `+0x6f9` = 1, quickbars null | `OnReadyToStartMatch` ran or is skipped at the guard | Check `0x143f7a920` and the path from `0x1419e6130` down to `0x14239d167` |
| More than one controller with a table in the top rows | The server replicated an extra controller | Compare `+0xf0` (role) of each |

## Open points

- Nothing here has been run against a live client. The first lobby scan is the test.
- Field offsets `+0x6f8`, `+0x6f9`, `+0x14d0` and `+0xf0` are confirmed in the disassembly. `+0x1738` and `+0x19e8`
  come from the earlier decompile and were not rechecked.
- The hero and character-parts theory (plan D1) is not ruled out. The probe result decides whether it is the
  cause or a separate defect.
