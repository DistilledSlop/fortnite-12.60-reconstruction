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

Verified local evidence is in `generated/material_pipeline/frontend-game-library.json`: the four currently failing frontend material files (`M_UI_DonutIcons`, `Athena_LockerTabMaster`, `M_MenuButton_Master`, `M-Border-PowerToastGlow`) each contain 38 exact shader-output-hash occurrences, identifying 44 unique library shaders together. All 44 payloads decode to their declared sizes and contain bounded DXBC containers (30 frequency-3, 12 frequency-0, two frequency-4). The original Global sample also decodes, and the rebuilt Vulkan library's 940 shaders/35 maps parse with the other layout. These initial exact byte matches are now confirmed against the decoded shader records for the four samples, as described below; they do not prove an executable draw.

The surviving material reports show stripped arithmetic nodes. Original bytecode provides a concrete recovery input. The serialized maps are decoded and their DXBC is translated as described below; a detached Locker runtime adapter and live numeric evaluation are now exercised as described below; a Locker quad draw is exercised below, while material attachment and visual comparison remain necessary. No root menu or visual-parity milestone is complete from this evidence.

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

`inspect_spirv_bindings.py` independently reads the validated SPIR-V binaries, extracts `DescriptorSet`/`Binding` decorations on variables, and places them beside the original D3D resource-table entries. It requires the same source-report identity and hashes each output before inspection. The generated correspondence report covers all 44 shaders and 180 SPIR-V descriptor records (100 constant-buffer, 54 texture, 26 sampler names); all observed sets are zero. This earlier inventory alone does not establish mapping because the compiler can renumber D3D registers; the explicit API mapping below supplies that correspondence. This inventory predates the detached Locker runtime bridge described below.

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

The script consumes the already generated reports and binaries, whose receipt paths are local to the workspace. It checks recorded successful validation status, but does not rerun `spirv-val`. This comparison alone does not establish runtime binding, uniform values, shader-map reconstruction, draw execution or visual parity; the subsequent detached Locker test is described below.

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

These reports establish the recorded register correspondence, not runtime semantics. The detached Locker bridge now exercises headers and live numeric values as described below; normal material attachment and visual comparison remain unresolved; the later Locker quad draw is described below.

## Reflection and numeric constant linkage

`reflect_mapped_spirv.py` reads the actual mapped modules: across 44 shaders it reports 100 uniform buffers, 28 uniform texel buffers, 26 sampled images and 26 samplers, along with declared constant-buffer spans and stage interfaces. This refines the earlier debug-name inventory. `evaluate_uniforms.py` evaluates serialized numeric defaults for the four material resources: Donut 96 bytes, Locker 320, MenuButton 288 and PowerToast 192. These match their serialized debug first-resource offsets. Six evaluator tests cover supported cases; floating-point behavior, instance overrides and resource values remain limitations.

`link_material_constants.py` links 76 parsed Slate and mesh records and 44 Material uniform-buffer bindings across the four materials through their original serialized slots to explicit descriptors and evaluated numeric bytes. Each declared span fits its evaluated buffer; the generated receipt is `generated/material_pipeline/material-constant-bindings.json`. Three linker tests cover this evidence. These defaults do not establish instance values, runtime resource binding, shader installation, execution semantics or visual parity.

An isolated software Vulkan probe used original geometry shaders 26 and 2459 to exercise a real `FVulkanShaderHeader` serialization round trip and the RHI shader factory; modified SPIR-V passed validation. It did not create a Vulkan shader module or pipeline or issue a draw. The successful `generated/frontend-probes/original-vulkan-probe-overlay.log` follows three retained graphics-startup exit-139 logs caused by stale default material/shader archive files in the baseline. Its `original-vulkan-stage/overlay-manifest.json` records 330 cooked Engine files and two matching archives overlaid into the isolated stage.

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


## Original serialized shader maps

`legacy_shader_map.py` decodes the original resource header, uniform expressions, virtual-texture and collection slots, compilation output, packed flags and shader records through exact EOF for all four resources. The texture parameter layout is `int32`, `int16`, `int16`, `int32`, `bool32`, totaling 16 bytes, as in the 4.24 layout; the earlier four-word 4.23 versus five-word 4.24 claim was incorrect. Each map contains 19 sequential shader records: seven Slate and twelve LocalVF, no pipelines, and cooking marker 1. Across the four maps, all 76 records have their archive hash, frequency and uncompressed size validated, representing 44 unique archive associations (15, 15, 14 and 14 unique per sample). Their union exactly matches the 44 originally recovered shader selections, confirming their association for these four samples. Donut, Locker, MenuButton and PowerToast resource byte counts are 8420, 12513, 10802 and 10312, ending at `.uexp` offsets 10601, 15310, 15332 and 14311 respectively. Full commands and counts are in `generated/material_pipeline/full-original-shader-map-validation.json`; `original-serializer-evidence.json` records immutable hashes and 29 decompilation files. Reports record `complete_serialized_shader_map=true`, `first_unknown=null`, zero undecoded bytes and `runtime_loading_verified=false`. Seventeen isolated tests cover the decoder.

Decoding alone does not establish runtime behavior. The implemented detached Locker bridge is described in [UE_BRIDGE.md](UE_BRIDGE.md). Its fixture `generated/material_pipeline/locker-runtime-fixture.json` (pin SHA-1 `3c379d3b0b97bb8e57e9b7b8b7286182ac78a700`) has 220 nodes, seven shader records and eight cases: defaults, four individual overrides, combined overrides, wrong association and wrong index. The cases check 312 meaningful numeric bytes within the 336-byte layout.

The actual non-editor software Vulkan/Xvfb run exited 0 and passed all eight live `FillUniformBuffer` cases, froze the detached map and constructed seven RHI shaders (`generated/frontend-probes/legacy-material-runtime-1.log`). The 14 emitted artifacts in `generated/frontend-probes/legacy-material-artifacts-1` were independently verified against the actual seven SPIR-V modules and payloads; the validation receipt is `generated/frontend-probes/legacy-material-artifacts-1-validation.json`. Instruction streams changed only for entry-point names; header offsets match actual descriptors; current layout hash 22544639 derives from the original shape; SourceHash, CRC and embedded code match. Three verifier tests pass. A subsequent actual Vulkan run drew a 64×64 Locker quad and read back two `RBGA1` overrides: 2,832 of 4,096 pixels differed, with red and green center pixels (`generated/frontend-probes/legacy-material-runtime-draw-1.log`, `generated/frontend-probes/legacy-material-artifacts-2/LockerQuad.json` and RGBA files). Eight pre-freeze and eight post-freeze uniform cases passed. At that detached checkpoint normal material attachment had not yet been tested; original visual parity remains unverified.

## Complete source bundle for asset ingress

```sh
python3 -I porting/material_pipeline/export_ingress_bundle.py
python3 -I porting/material_pipeline/test_export_ingress_bundle.py
```

The separate `generated/material_pipeline/locker-ingress-bundle.json` preserves
the existing runtime fixture while adding the original resource bytes, local
names, complete decoded report, all 19 shader-library associations and hashes
of the original code payloads. The exporter rereads the original `.uexp` and
shader archive, recomputes the wrapper and full parse, and checks the runtime
graph and mapped modules against their receipts. Its initial SHA-1 is
`0f750204f6008e6e3219d0b002d8409e28c22426`; the original 12,513-byte
resource has SHA-1 `bc2d2f509b6909a4820f65188b85485fce989a2a`.

Four tests cover changed source bytes, export bounds, local name/resource
identity and complete parameter identity. An original-input negative run also
rejects a corrupted mesh shader record. The bundle supports independent parsing
of live archive bytes; it does not prove loading, completeness or attachment.
The serialized owner properties in `generated/frontend-probes/material-locker.json`
explicitly record `MD_UI`, translucent blending and unlit shading. Actual-owner
shader requirements still need checking in the runtime described by
[the ingress plan](UE_MATERIAL_INGRESS.md).

The subsequent actual-asset bridge is now verified for Locker: native `c8ec113`
and Engine `e6e962a43` load the original owner, retain all 19 source records,
require seven shaders with zero missing, attach the same map on both threads
and draw its quad. Owner-3 and independent artifact receipts are under
`generated/frontend-probes/legacy-material-ingress-*`; the full command and
remaining material, UI-root and original-comparison work are documented in
[UE_MATERIAL_INGRESS.md](UE_MATERIAL_INGRESS.md). All four product gates remain open.

## Numeric asset ingress checkpoint

Engine `6925baad3` and native `25bf1c4` extend the verified actual-owner path to the original GradientPanel base and PowerToastGlow instance. Each retains 19 original records, converts seven and has zero missing required shaders; eight graph and eight actual-owner uniform cases pass, and the attached quad changes 3,952 pixels. Five source instance parameter override identities were verified. Both PowerToast and Locker pass independent verification of all seven artifacts. Client/server builds and fresh default/Athena joins pass. Exact commands, hashes and receipts are in `generated/material_pipeline/numeric-power-checkpoint-results.json`. Six graph dependencies were recovered verbatim; owner-package import warnings are absent and preexisting Water startup warnings remain. These tests do not establish original pixels, Editor rebuild or full UI parity.

The CUE report now preserves full scalar/vector `ParameterInfo` identities; `export_numeric_ingress.py` produces immutable v2 bundles from the original source and owner report, without changing the original Locker pin. It requires `--key` and `--owner-report` and writes to a fresh generated path. See [UE_MATERIAL_INGRESS.md](UE_MATERIAL_INGRESS.md) for multiple-bundle invocation and exact owner selection. MenuButton's original View byte 2264 is identified as GameTime in [view_original_identity.md](view_original_identity.md), while native offset and runtime binding remain unverified.
