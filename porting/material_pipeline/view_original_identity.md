# Original View byte 2264 is GameTime

The supplied runtime dump proves that the MenuButton5 Slate pixel shaders' View
read at byte 2264 is `FViewUniformShaderParameters.GameTime`, a scalar float.
It is not inferred from the current UE member order.

Original static constructor `0x140555220` walks the member-helper chain starting
at `0x144509e50`, reverses the records, and registers
`FViewUniformShaderParameters` / `View` at metadata object `0x148342b80`, with
struct size `0xf70`. The recovered chain has 229 helpers. Its helper at reverse
index 162, `0x144502fe0`, constructs the member record:

- Name pointer `0x146aadc50`: UTF-16 `GameTime`.
- Instruction `0x144503004`: stores offset `0x8d8` (2264).
- Instruction `0x14450301f`: stores base-type byte 4 (`UBMT_FLOAT32`).
- Instructions `0x144503024` and `0x14450302c`: rows 1, columns 1, elements 0.
- Next helper `0x144500ff0`: `AdaptiveTessellationFactor`, offset `0x8d4`.
- Preceding helper `0x144505060`: `RealTime`, offset `0x8dc`.

The mapped record 6854 SPIR-V independently reads float32 from `cb1_1`, set 0,
binding 1, array index 141, lane 2: `141*16 + 2*4 = 2264`. The declared block
span is 2272 bytes; this is a shader-visible prefix, not the original full View
struct size. The bounded instruction evidence includes scalar loads and vector
loads whose only selected lane is 2.

Current Engine `SceneView.h` declares `GameTime` as float. Its metadata macro
passes `STRUCT_OFFSET(zzTThisStruct, MemberName)` and the float type's alignment
is 4. `view_current_layout.py` derives the current member prefix from those
actual declarations and type-info metadata: `GameTime` 2264, `RealTime` 2268.
This calculation is source-derived; it does not claim to compile the Engine.
`SceneView.cpp:2360` assigns `GameTime = Family->CurrentWorldTime`; `RealTime`
uses `CurrentRealTime` separately. The native bridge should verify the actual
member metadata or `STRUCT_OFFSET` when compiled, and bind the real current
view uniform buffer. This evidence does not authorize a substitute timer,
synthetic zero buffer, or a claim of rendered parity.

Reproduction (run in the client repo):

```sh
porting/.venv/bin/python -I porting/material_pipeline/view_original_probe.py --strings 146aadc50 146aadc68 --functions 140555220 --chain 144509e50 --out porting/generated/material_pipeline/view-identity
python3 porting/material_pipeline/view_current_layout.py --out porting/generated/material_pipeline/view-identity/current-layout.json
```

The ignored `view-identity/evidence.json` records dump/SPIR-V/source hashes,
original VA byte slices and exact SPIR-V operation word offsets. The same
folder contains the complete helper chain and bounded decompilation/disassembly.
Dump SHA-256: `604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
The Ghidra script never saves the program or starts an editing transaction.
Opening/closing the project updates Ghidra index/property bookkeeping; the
observed changes are reported in `project-integrity.json`.
