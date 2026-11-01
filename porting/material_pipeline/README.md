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

All 44 selected original DXBC shaders compiled and passed Vulkan 1.0 SPIR-V validation. **Twelve jobs emit `shader_sm4_read_instruction_modifier` diagnostics for unhandled resource/data modifiers with this compiler version.** The receipt retains the exact messages and `compiler_diagnostic_jobs` count. The later comparison below examines these modifiers with vkd3d 2.0; zero compile/validator exits do not prove semantic preservation. Original uniform values, descriptor bindings, vertex interfaces, render state, and inline shader-map reconstruction still require implementation and visual tests. No output is yet verified in a UE draw.

## Original D3D wrapper bindings

`inspect_d3d_code.py` parses the original wrapper before and after the DXBC container. Its prefix follows the local engine's `FD3D11ShaderResourceTable` serializer (`RenderCore/Public/ShaderCore.h`, `Windows/D3D11RHI/Public/D3D11ShaderResources.h`): resource-table bits, SRV/sampler/UAV/layout-hash arrays, then the texture map. It decodes packed resource entries using `RHI/Public/RHIDefinitions.h`. Its suffix follows `FShaderCodeReader`'s optional-data framing and decodes the packed resource counts and uniform-buffer names. Every boundary must align exactly with the DXBC container from the verified source report; changed wrapper hashes are rejected.

```sh
python3 -I porting/material_pipeline/inspect_d3d_code.py \
  --source-report porting/generated/material_pipeline/frontend-game-library.json \
  --report porting/generated/material_pipeline/d3d-wrapper-inspection.json
python3 -I porting/material_pipeline/test_inspect_d3d_code.py
```

All 44 recovered wrappers pass exact prefix/DXBC/optional framing. Nineteen have active resource-table bits; the other 25 have empty maps. Across the 44, the decoded maps contain 28 SRV, 18 sampler, 18 texture, and zero UAV entries. Optional `p` and `u` records are present in each. The evidence report preserves each original map and decoded entry, including layout hashes and uniform-buffer names. Those entries are D3D bindings; explicit compiler API mapping is documented below, but they have not been installed in UE's Vulkan RHI. The same 12 shaders with compiler diagnostics all have active D3D resource-table bits, so their semantics and binding correspondence need direct validation before rendering claims.

`inspect_spirv_bindings.py` independently reads the validated SPIR-V binaries, extracts `DescriptorSet`/`Binding` decorations on variables, and places them beside the original D3D resource-table entries. It requires the same source-report identity and hashes each output before inspection. The generated correspondence report covers all 44 shaders and 180 SPIR-V descriptor records (100 constant-buffer, 54 texture, 26 sampler names); all observed sets are zero. This earlier inventory alone does not establish mapping because the compiler can renumber D3D registers; the explicit API mapping below supplies that correspondence. The UE Vulkan shader header, uniform values, and shader-map registration are still absent.

```sh
python3 -I porting/material_pipeline/inspect_spirv_bindings.py \
  --d3d-report porting/generated/material_pipeline/d3d-wrapper-inspection.json \
  --translation-report porting/generated/material_pipeline/frontend-translated/translation-report.json \
  --report porting/generated/material_pipeline/binding-correspondence.json
python3 -I porting/material_pipeline/test_inspect_spirv_bindings.py
```


## Compiler version comparison

The existing receipts also cover a locally built official Wine vkd3d 2.0 compiler from [vkd3d 2.0 source](https://dl.winehq.org/vkd3d/source/vkd3d-2.0.tar.xz) (source tar SHA-256 `9ad29bb236808186a47ec66e853b21e6fca59b9dc62a9474b05d5e3eda2710ef`). The isolated build lives under ignored `generated/material_pipeline/tools/build-vkd3d2`; its compiler SHA-256 is `962b4898d5ceb71b20ada3df4c3da2130bb48ea8959425896e2e59724f4ed0d7` and shader library SHA-256 is `481674c3dd9266a85e3504cf41bf70617431daf916bc3d94e66997c361a91571`. Build dependencies were unpacked locally under `tools/root` and no system package was installed; a temporary `/usr/bin/m4` symlink used during generation was removed. The same 44 input hashes compile and validate for Vulkan 1.0 with both versions. The 1.2 receipt records diagnostics on shader indices 25, 116, 117, 120, 2452, 2453, 2456, 2458, 2466, 3692, 6859, and 13209; the 2.0 receipt records none. These receipts contain hashes of each compiler, shader library and validator. A successful validation and absence of diagnostics do not establish semantic equivalence.

Recreate the 2.0 receipt from the existing isolated build and verified original DXBC inputs:

```sh
python3 -I porting/material_pipeline/translate_dxbc.py \
  --source-report porting/generated/material_pipeline/frontend-game-library.json \
  --tool-prefix porting/generated/material_pipeline/tools/vkd3d2-compiled \
  --output porting/generated/material_pipeline/frontend-translated-vkd3d2
python3 -I porting/material_pipeline/inspect_spirv_bindings.py \
  --d3d-report porting/generated/material_pipeline/d3d-wrapper-inspection.json \
  --translation-report porting/generated/material_pipeline/frontend-translated-vkd3d2/translation-report.json \
  --report porting/generated/material_pipeline/binding-correspondence-vkd3d2.json
```

In the 2.0 `libs/vkd3d-shader/tpf.c` parser, resource type `0xc` is `VKD3D_SM4_RESOURCE_STRUCTURED_BUFFER` and data type `0x6` is `VKD3D_SM4_DATA_MIXED`. The new parser marks the former structured and maps it to a buffer resource, and maps the latter to `VSIR_DATA_MIXED`. The 1.2 diagnostic explicitly says it did not handle these values; that establishes an older parser limitation, not by itself the observable impact of the fallback.

The following comparison checks the input and output hashes against both receipts and their binding reports, then disassembles one bounded example. Across all 44, each shader has the same multiset of descriptor set, binding, and storage class values in the two inventories (180 descriptors each). This comparison does not establish descriptor types or D3D-to-Vulkan mapping. For shader 25 specifically, both outputs declare unsigned integer and float buffer images; the `OpTypeImage` depth operand changes from `0` to `2`, and both contain 16 unsigned and three float `OpImageFetch` instructions. Counts and declarations do not establish equal addressing, fetched values, or execution semantics. Other instructions and shaders have not been exhaustively compared.

```sh
python3 -I porting/material_pipeline/compare_vkd3d_reports.py \
  --old porting/generated/material_pipeline/frontend-translated/translation-report.json \
  --new porting/generated/material_pipeline/frontend-translated-vkd3d2/translation-report.json \
  --old-bindings porting/generated/material_pipeline/binding-correspondence.json \
  --new-bindings porting/generated/material_pipeline/binding-correspondence-vkd3d2.json \
  --spirv-dis porting/generated/material_pipeline/tools/root/usr/bin/spirv-dis
```

The script consumes the already generated reports and binaries, whose receipt paths are local to the workspace. It checks recorded successful validation status, but does not rerun `spirv-val`. The 2.0 outputs are not installed in UE, and bindings, uniform values, shader-map reconstruction, draw execution, and visual parity remain unverified.

## Explicit register mappings

`translate_mapped_dxbc.py` uses the vkd3d 2.0 scan and explicit interface APIs to map original D3D registers to SPIR-V descriptors. The hash-checked `frontend-mapped/mapped-translation-report.json` records 44 validated shaders, 180 descriptors, 64 matched original resource-table entries and zero unmatched entries or compiler diagnostics. It records the exact SPIR-V `DescriptorSet` and `Binding` literal word offsets needed for UE remapping. An independent audit checked hashes, offsets and table counts. A separate `frontend-mapped-offset` run for shader 25 with binding base 32 differs from its base-zero output only in five `Binding` literal words.

```sh
python3 -I porting/material_pipeline/translate_mapped_dxbc.py \
  --source-report porting/generated/material_pipeline/frontend-game-library.json \
  --d3d-report porting/generated/material_pipeline/d3d-wrapper-inspection.json \
  --output porting/generated/material_pipeline/frontend-mapped
python3 -I porting/material_pipeline/translate_mapped_dxbc.py \
  --source-report porting/generated/material_pipeline/frontend-game-library.json \
  --d3d-report porting/generated/material_pipeline/d3d-wrapper-inspection.json \
  --output porting/generated/material_pipeline/frontend-mapped-offset \
  --binding-base 32 --limit 1
```

These reports establish the recorded register correspondence, not runtime semantics. UE shader headers, runtime uniform values, resource binding, shader-map installation, real draws and visual comparison remain unresolved.

## Original inline material framing

`Cue4Inspect --export-map-json` reads the legacy package's export table without relying on guessed `.uexp` positions. `inspect_material_resource.py` then scans only the selected export for the exact `FMaterialResourceMemoryWriter` wrapper: serialized name entries, resource locations, byte count, and a payload ending exactly at the export boundary. It requires one unambiguous match and records SHA-256 identities. The scanner also checks the observed cooked/valid, quality/feature, 20-byte shader-map ID hash, and two length-prefixed material identity strings at the start of the resource payload. The word after the hash is recorded without assigning it an unverified field meaning.

Example, with the built tool's path adjusted to the current isolated output:

```sh
dotnet porting/generated/frontend-probes/cue4inspect-export-map/Cue4Inspect.dll \
  /Game/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons \
  /mnt/vibrato/fortnite-extracted-full --export-map-json --loose \
  > porting/generated/material_pipeline/donut-export-map.raw
python3 -I porting/material_pipeline/inspect_material_resource.py \
  --uexp /mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons.uexp \
  --export-map porting/generated/material_pipeline/donut-export-map.raw \
  --report porting/generated/material_pipeline/donut-inline-resource.json
python3 -I porting/material_pipeline/test_inspect_material_resource.py
```

| Original package | Export `.uexp` range | Wrapper start | Resource range | Name entries | Identity in resource |
| --- | ---: | ---: | ---: | ---: | --- |
| `M_UI_DonutIcons` | 0..10601 | 683 | 2181..10601 | 37 | `M_UI_DonutIcons` |
| `Athena_LockerTabMaster` | 0..15310 | 1037 | 2797..15310 | 46 | `Athena_LockerTabMaster` |
| `M_MenuButton_Master` | 0..15332 | 2566 | 4530..15332 | 54 | `M_MenuButton_Master` |
| `M-Border-PowerToastGlow` | 0..14311 | 2299 | 3999..14311 | 44 | parent `M_UI_GradientPanel` |

These are the four material packages reached by the failing frontend probes. Each has one resource location (offset 0, feature level 3, quality level 1), and its declared byte count ends at its export boundary. In all four, the local engine's `FMaterialShaderMapId::Serialize` would interpret the zero word immediately after the original 20-byte hash as `MaxFieldAlignment` and the following material-name length (16, 23, 20, or 19) as layout flags. The local layout structure requires an initialized flag and usable alignment; it then reads the material-name bytes as `FrozenContentSize`, explaining the observed `0x49555f4d` (`M_UI`) read for Donut. This establishes the first format divergence at the material shader-map ID/base boundary. It does **not** establish the full original shader-map schema or make Windows-cooked shader maps usable by the Linux Vulkan runtime. The exact wrapper bounds replace the earlier inferred 205-byte origin; archive `Tell` and `.uexp` positions use different coordinate systems in the captured trace.


## Pre-freeze uniform expression prefix

`legacy_shader_map.py` strictly parses the observed original resource header,
eight uniform-expression arrays, the intervening virtual-texture stack slot,
parameter collections, and the compilation output scalar prefix. It checks
package and resource identities against the framing report. All four original
resources have zero virtual-texture stacks and zero parameter collections.
The Donut texture parameter follows the older four-word 4.23 texture
serializer; reading only two words produced a false boundary. The parser stops
before packed compilation flags: the public 4.24 serializer's following
debug-string boundary does not match the original bytes. It does not
decode shader records or reconstruct a runnable shader map. Ten tests cover
the supported prefix and malformed input. Reports under
`generated/material_pipeline/*-legacy-prefix.json` record these boundaries:

| Resource | Expression nodes | First unknown resource-relative offset | First unknown `.uexp` offset |
| --- | ---: | ---: | ---: |
| Donut | 13 | 546 | 2727 |
| Locker | 220 | 4613 | 7410 |
| MenuButton | 124 | 2840 | 7370 |
| PowerToast | 81 | 1906 | 5905 |

These boundaries are evidence for continuing the parser, not proof of uniform values, resource binding correspondence, shader semantics, or UE draw parity.
