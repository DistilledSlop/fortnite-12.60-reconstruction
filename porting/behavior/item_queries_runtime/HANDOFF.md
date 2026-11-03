# UFortItem query base methods

Finite proposal: `manifest-entries.json` and six `.inc` files, reproduced with
`python3 -I porting/behavior/item_queries_runtime/prepare.py` (six proven).
The script uses image-125 and the existing hardened `proven_this_store` proof;
`build_vtable_slots.py` resolves GPSC `0x142951e90` -> thunk `0x1429551e0`
-> constructor `0x142024010`, whose single primary vptr store to original
`this` installs `0x14606c3f0`. Exact wrapper instruction sequences, reflected
parameter types, slots, and targets are asserted. The object return's class is
resolved via `class_ref_map.json` to `UFortItemDefinition`.

| Method | Exec | Slot | Base target | Result |
|---|---|---|---|---|
| GetDurability | 0x142958db0 | 0x2c8 | 0x14084bee0 | -1.0f |
| GetNormalizedDurability | 0x1429594c0 | 0x2d0 | 0x1407c46d0 | 1.0f |
| GetNumInStack | 0x142959500 | 0x368 | 0x140833bf0 | 1 |
| GetNumberOfSlotsTaken | 0x142959530 | 0x2e8 | 0x1406801d0 | uint8 1 |
| GetRemainingAmmo | 0x142959880 | 0x380 | 0x140693f70 | -1 |
| GetItemDefinitionBP | 0x142959140 | 0x3e8 | 0x140680300 | nullptr |

Float functions load constants from `0x1459a979c` and `0x1459a86f8` into
XMM0; script checks float values and instructions. Typed decompilation alone
misrepresents those float returns as integer bits; wrapper disassembly verifies
the ABI. Original vtable dispatch requires `virtual: true` in each proposal.
These are complete **base implementations**, not implementations of derived
inventory item overrides. Derived overrides require independent slot and body
recovery before claiming complete inventory behavior. All six are source
proposals only, with no canonical promotion, build, or runtime validation.

Ghidra evidence: `porting/generated/callee_probe/item_queries_1/` wrappers and
`item_queries_2/` slot targets, from `porting/.venv/bin/python -I
porting/ghidra/probe_callees.py --input porting/behavior/item_queries_runtime/targets.json
--out porting/generated/callee_probe/item_queries_1` and the same command using
`slots.json` and output `item_queries_2`. Both completed. Probe reports project
metadata files changed; its analysis transaction was rolled back. Evidence dirs
are generated and ignored. No tests or builds, per current batch cadence.

Review hardening: `prepare.py` verifies the entire image-125 SHA-256 against
`bulk_virtual_scalar_runtime/prepare.py`'s expected digest before mapping or
decoding. Both four-byte float sources must lie fully within a readable,
initialized-data PE section without `IMAGE_SCN_MEM_WRITE`. Exact reflection
function and return-property flags, return kinds, raw kinds and type references
are checked; a Byte with an enum type reference is rejected. Existing ported
manifest entries, maintained definitions and generated UFortItem declarations
or definitions are rejected. Re-running the command above completed with six
proposals after these checks; no build or runtime tests were run.

Integrated in native source batch 19: six base virtual implementations. Compilation/runtime checks remain pending at approximately 2,040 bodies.
