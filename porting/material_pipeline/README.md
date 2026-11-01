# Original cooked shader recovery

`shader_archive.py` reads the supplied shader libraries without changing them, finds exact library-hash candidates in material `.uexp` bytes, and exports a bounded selection of the original shader wrappers and DXBC containers. It does not create replacement material graphs or claim that DXBC can render through the Linux Vulkan RHI.

The original libraries and the rebuilt project's libraries both begin with version `2`, but their layouts differ:

| Explicit layout | Index | Compression |
| --- | --- | --- |
| `fortnite-12.60` (default) | Signed count followed by 29-byte records: SHA1 output identity, compressed size, uncompressed size, frequency. Offsets are cumulative. Payload ends exactly at EOF. | zlib for the supplied archives |
| `ue4-serialized` | The six arrays in UE 4.25 `RenderCore/Public/ShaderCodeArchive.h`: map hashes, shader hashes, map entries, code entries, preload entries, shader indices. | Raw or LZ4 according to size |

The default layout is grounded in the supplied bytes, not selected from the version word alone. Both complete original indexes pass bounds, uniqueness and exact-EOF checks. The FortniteGame library contains 213,239 shader records, payload offset 6,183,939; Global contains 1,899 records, payload offset 55,079. Public UE 4.25's `ShaderCodeArchive.cpp` describes the second layout and its LZ4 decoder; that schema cannot be applied to these original files merely because their version word is also `2`.

```sh
python3 -I porting/material_pipeline/shader_archive.py \
  --archive /mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode \
  --material-uexp /mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons.uexp \
  --max-shaders 64 \
  --extract-dir porting/generated/material_pipeline/donut \
  --report porting/generated/material_pipeline/donut.json
python3 -I porting/material_pipeline/test_shader_archive.py
```

Use repeated `--material-uexp` arguments, a bounded `--shader-index`, or `--map-hash` for the serialized layout. Extraction and reports must stay inside `porting/generated`. The default eight-shader limit is explicit in `selected_shader_count`, `decoded_shader_count`, and `selection_truncated`. Only files named in a successful report are authoritative; old files in an output directory are not selected automatically. Report paths are local to this workspace; archive/material SHA-256 and library output identities establish byte identity independently of those paths. A library output identity is not asserted to equal SHA1 of the decompressed wrapper.

Verified local evidence is in `generated/material_pipeline/frontend-game-library.json`: the four currently failing frontend material files (`M_UI_DonutIcons`, `Athena_LockerTabMaster`, `M_MenuButton_Master`, `M-Border-PowerToastGlow`) each contain 38 exact shader-output-hash occurrences, identifying 44 unique library shaders together. All 44 payloads decode to their declared sizes and contain bounded DXBC containers (30 frequency-3, 12 frequency-0, two frequency-4). The original Global sample also decodes, and the rebuilt Vulkan library's 940 shaders/35 maps parse with the other layout. These are exact byte matches, not yet a decoder of the inline material fields or proof that every candidate belongs to an executable draw.

The surviving material reports show stripped arithmetic nodes. Original bytecode therefore provides a concrete recovery input, but inline material/shader serialization, uniform/resource bindings, DXBC-to-Vulkan translation, actual draw execution, and visual comparison all remain necessary. No root menu or visual-parity milestone is complete from this evidence.

## Verified SPIR-V translation

`translate_dxbc.py` verifies each recovered wrapper hash and checks the DXBC bytes against its exact wrapper slice before invoking the compiler. It publishes a SPIR-V file only after `spirv-val --target-env vulkan1.0` succeeds, with compiler/library/validator and input/output hashes in the receipt. Partial or failed files are not published as validated outputs. Use the receipt, not an output glob, to consume successful jobs.

The local tools are unpacked Ubuntu packages under ignored `generated/material_pipeline/tools/root/usr`; no system packages were installed. Versions tested: `vkd3d-compiler` and `libvkd3d-shader1` `1.2-15build1`, `spirv-tools` `2026.1-1`. Their `.deb` files remain in `tools/packages`. The [Ubuntu compiler reference](https://manpages.ubuntu.com/manpages/resolute/man1/vkd3d-compiler.1.html) documents its DXBC source and target options; the actual installed tool lists `spirv-binary` support.

```sh
python3 -I porting/material_pipeline/translate_dxbc.py \
  --source-report porting/generated/material_pipeline/frontend-game-library.json \
  --output porting/generated/material_pipeline/frontend-translated
```

All 44 selected original DXBC shaders compiled and passed Vulkan 1.0 SPIR-V validation. **Twelve jobs emit `shader_sm4_read_instruction_modifier` diagnostics for unhandled resource/data modifiers with this compiler version.** The receipt retains the exact messages and `compiler_diagnostic_jobs` count. They require investigation with a compiler that handles the observed modifiers; zero compile/validator exits do not prove semantic preservation. Original uniform values, descriptor bindings, vertex interfaces, render state, and inline shader-map reconstruction still require implementation and visual tests. No output is yet verified in a UE draw.
