# Original texture index context and reference-array rebuild

The original texture index is an index into an ordered runtime reference array.
It is not an import-table index. The original parent material rebuilds this array
from its stored expression list during PostLoad, including referenced material
functions. The bounded ComicBubble parent mapping is now proven: index 0 is
T_TilingClouds_02 and index 1 is T_UI_Bubble_Tail. ComicBubble_Inst rebuilds that
same two-entry array and uses only index 0 in its compiled static permutation.
This establishes reference identity, not texture conversion or runtime compatibility.

Primary local evidence is
`generated/material_pipeline/original-texture-context-evidence.json`, SHA-256
`f997c42d5793527f968f193dd0e875b77e348729c0198e6c2a925401ce5ed0a0`.
It pins 69 decompilation, disassembly, index and integrity artifacts and checks
six vtable entries against the original runtime image. The dump SHA-256 is
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
This is a **memory-layout** image: VA = `0x140000000 + file offset`.
Its stale PE raw-section headers must not be used to locate code or pointers.
The first receipt and its artifacts remain immutable. The subsequent proof is
`generated/material_pipeline/original-texture-context-evidence-v2.json`, SHA-256
`74fb88c37e9ff5af49d207707c34abaa7df417da3d2efbbb25428d7f55e3465b`.
It pins 75 additional artifact/tool identities, checks nine additional original
vtable entries, and retains the earlier receipt as an input.
All 75 declared v2 artifacts also have immutable copies bound by
`generated/material_pipeline/original-texture-context-evidence-v2-frozen-binding-1.json`,
SHA-256 `4d51ddd8cc75f6fa72263cc201b218d26161f494e59434fa63ce0ad098c64ebd`.
The replay's subsequent function root-index/name guard is pinned with immutable
source copies and proof output in `original-texture-context-evidence-v3.json`,
SHA-256 `5617b511c2abe2553c8186182eec24f548fdff4872fbd366bea70f8c56f8cb08`.
The v2 replay source was reconstructed to its exact declared SHA-256
`f7ea1cc851b082a3fd724be600d44c813b0707cb8bc0374ea41f2190485a5e49`
before freezing; its working path had advanced. The v2 receipt bytes and
recovered artifact bytes remain unchanged. Future audits must use the frozen
binding for historical tool identities.

## Texture lookup

`original-texture-context-1/14433eda0.{c,asm}` establishes regular Texture
vtable `0x146ac1c08 + 0x50`. The evaluator first returns an expression-local
texture pointer at `this+0x28` when non-null. Otherwise it calls
`0x144342050(material, int32(this+0x10))`. TextureProperty supplies this material
argument from `context+8`; its original callsite is retained in the earlier
`expression-texture-proof-2/14433e4b0.{c,asm}`.

`original-texture-context-2/144342050.{c,asm}` proves the indexed lookup:

1. Call material virtual `+0x350` to obtain an array header.
2. Require `0 <= index < int32(header+8)`.
3. Read the object pointer at `*(header)+8*index`.
4. Require a non-null object whose class passes the original UTexture class
   hierarchy check through `0x144804790`.
5. Return null on failure. Invalid index/null entry can emit the original
   one-time `Requesting an invalid TextureIndex!` diagnostic.

TextureParameter vtable `0x146ac0d98 + 0x50 = 0x14433f6c0` uses the same local
pointer first. Otherwise it asks the context render proxy (`context+0`, virtual
`+0x70`) for the parameter at `this+0x30`. If unresolved, a runtime setting can
enable a material-interface default lookup (material virtual `+0x340`, interface
virtual `+0x378`). Failure uses the same indexed resolver. These paths establish
lookup precedence; they do not supply missing package references.

## Which array the original uses

`original-texture-context-6/14430b4f0.{c,asm}` allocates and initializes a
0x38-byte material resource with vtable `0x1467b70f0`. Its `+0x350` entry is
`0x1443298c0`, retained in `original-texture-context-8/`:

- If resource `+0x30` points to an instance and its array count at instance
  `+0x150` is nonzero, return the instance array header at `+0x148`.
- Otherwise use the resource's parent material at `+0x28`, or call
  `0x14430ef60(0)` for the default material if absent, and return that material's
  array header at `+0x2d0`.

This is the original layout. The current UE 4.25 `CachedExpressionData` layout
is not evidence that the original cooked asset serialized that structure.
An empty CUE selected-report `ReferencedTextures` is consequently not evidence
that the original runtime reference array was empty.

## Parent array construction

UMaterial PostLoad virtual `+0x70 = 0x144314eb0` is retained in
`original-texture-context-7/`. After its render-capability check
`0x14089d900` and material flag check (`material+0x211`, bit 3 clear), it clears
the `+0x2d0` array, invokes material virtual `+0x3a0` with that array, postloads
the resulting non-null textures, and continues resource initialization.
`original-texture-context-12/14089d900.c` includes the original `nullrhi` switch
check. UMaterial's structured Serialize `0x144316c60`, retained in
`original-texture-context-10/`, has no direct serialization of `+0x2d0`.
The positive evidence here is the runtime rebuild, not a claim that every base
or reflected serialization path has been exhaustively excluded.

Material virtual `+0x3a0 = 0x14430b550` is retained in
`original-texture-context-11/`. It traverses the Expressions array at
`material+0x1e0`, count `+0x1e8`, in increasing stored-array order:

- A `MaterialExpressionMaterialFunctionCall` with a non-null function pointer
  at expression `+0x40` traverses that function through its virtual `+0x288`,
  builds a dependency list, then calls each dependency's virtual `+0x298` to
  append its referenced textures.
- A `MaterialExpressionMaterialAttributeLayers` uses helper `0x14430efc0` to
  traverse its layer and blend function arrays, then calls virtual `+0x298`
  on each collected function. The helper's original code is retained in
  `original-texture-context-12/`.
- Other non-null expressions provide an object through virtual `+0x268` and
  a reference-capability predicate through `+0x270`. When the predicate is true,
  the object is appended only if pointer equality finds no existing entry.
  Thus first occurrence determines the index in this branch.

The two subclass checks are identified by original class registrations
`0x14478b840` and `0x14478ba90`, whose explicit Engine class-name literals are
retained in `original-texture-context-12/`. The additional constructor callback
probes in directories 14 and 15 were read-only in memory; directory 14 created
missing function definitions inside a transaction that was rolled back.

## Function traversal and reference capability

Original registrations, constructor callbacks and constructors in directories
16–18 establish TextureSample vtable `0x146aaf618` and MaterialFunction vtable
`0x146ab03e0`. Directory 19 proves:

- TextureSample `+0x268 = 0x140e54530` returns its Texture pointer at `+0x40`;
  `+0x270 = 0x1406801d0` returns true.
- MaterialFunction `+0x288 = 0x144314400` traverses its FunctionExpressions array
  at `+0x50`, count `+0x58`, in stored order. It recursively traverses each
  retained MaterialFunctionCall's non-null function at `+0x40`, then invokes
  the callback for that function: dependency postorder.
- MaterialFunction `+0x298 = 0x14430b7c0` walks that same expression array and
  **appends** every reference-capable expression's returned object, without
  deduplicating texture pointers. Do not apply the parent's direct-expression
  AddUnique rule to this function branch.

The parent callback helper `0x14085a990`, retained in directory 20, does
AddUnique on the *function list*, preserving first occurrence. Parent code then
adds the called function itself with the same helper. The function list is
local to each parent function-call expression.

Directories 21–23 follow explicit original class registrations to constructor
callbacks and constructors for the remaining ComicBubble retained classes.
ScalarParameter vtable `0x146ab2418`, VectorParameter `0x146ab2940`, and
StaticSwitchParameter `0x146ab26b0` all have `+0x270 = 0x1406801e0`, which returns
false (directory 24 and original bytes `32c0c3`). MaterialFunctionCall's actual
constructor `0x144306a50`, retained in directory 15, installs vtable
`0x146aae6d8`; its `+0x270` also returns false. This proves the skipped classes'
reference behavior from the original executable.

## ComicBubble parent mapping

The pinned inventory's stored Expressions list places the three retained
TextureSample nodes at zero-based positions 25, 27 and 34. Their direct defaults
are T_TilingClouds_02, T_UI_Bubble_Tail and T_UI_Bubble_Tail. Function-call nodes
occur before, between and after them, including at positions 5, 8 and 13.
Functions could therefore have appended references before the first sample;
checking the closure is necessary.

The isolated `original_texture_function_probe/` utility dumps generic original
FunctionExpressions properties, including null slots, and hashes both raw
package files. It references existing compiled CUE dependencies and writes its
own build outputs, without changing or rebuilding the shared Cue4Inspect tool.
All seven imported/dependency function packages are retained under
`generated/material_pipeline/comicbubble-function-properties-1/`. Five have only
null FunctionExpressions entries. ScaleUVsByCenter has one retained function
call to BreakOutFloat2Components; MF_SquareEdgeMask has one to LinearGradient.
All non-null entries are original MaterialFunctionCalls, whose reference
capability is false. Every visited function therefore appends zero textures.

`prove_comicbubble_texture_context.py` replays the proven parent and function
ordering against those reports. It checks source hashes, the original parent
export-map index/name pairs, duplicate reports, duplicate expressions, explicit
FunctionExpressions arrays, dependency completeness and cycles. Function targets
must match the dependency report's actual root export index, type and name;
top-level typed object names must match the corresponding root. It rejects
unknown retained classes, layers, and texture-bearing function expressions.
The receipt pins the parent raw `.uasset/.uexp`, export map, selected report and
complete decoded resource. Its output
`generated/material_pipeline/comicbubble-texture-context-proof-4.json` proves:

| Reference index | Original texture object |
| --- | --- |
| 0 | `FortniteGame/Content/Packages/Fortnite_UniqueMaterials_1/PostProcess/T_TilingClouds_02.T_TilingClouds_02` |
| 1 | `FortniteGame/Content/Environments/FrontEnd/BackPlates/UITakeovers/T_UI_Bubble_Tail.T_UI_Bubble_Tail` |

The two compiled regular Texture uniforms occur in reverse reference order:
uniform position 0 uses index 1 (tail), sampler source 2; uniform position 1
uses index 0 (cloud), sampler source 0. Both have layer/page -1 and virtual false.
The complete original 19-record resource SHA-256 is
`5872292505951dccccc70f3475916f104d9fac1cc6a07df1ce447a7d4c5f8767`.

The isolated .NET tool builds with zero warnings/errors. The original replay and
seven rejection checks pass: unproven function texture expression, missing
dependency, altered source hash, missing FunctionExpressions property and
duplicate function report, wrong function root export index and wrong function
root name. No runtime fixture or Linux texture conversion was
produced. The subsequent bounded instance proof below does not generalize the
mapping to arbitrary instances. Texture-bearing functions, layers, texture
parameters and TextureProperty resource dimensions need further work for the
remaining owners.

## ComicBubble_Inst array construction

`generated/material_pipeline/original-texture-context-evidence-v4.json`, SHA-256
`95b4887076fbda3e278d53e8de93a0fc6063932e77e862928de22cd941166c5b`,
pins 64 immutable snapshots for the subsequent instance investigation. The
receipt includes original class registration, constructor, PostLoad, array
writer and property-layout evidence, plus the isolated inspection tool source
and executable, raw-source-bound instance properties and bounded replay.

Original MaterialInstance/Constant class registrations in directory 25 lead
through callbacks in directory 26 to constructors in directory 27. Constant
constructor `0x14431cc10` installs vtable `0x146abc450`; its base constructor
`0x14431ca10` installs `0x146abba10` and initializes both the reference array at
`+0x148/+0x150` and layer-parameter array at `+0x188/+0x190` to zero.
Constant PostLoad `0x144336890` jumps directly to base PostLoad `0x144336390`.
The base invokes static-resource initialization `0x144321a10`, whose path calls
`0x1443225d0`. This writer clears `+0x148`, then fills it through virtual
`+0x3a0 = 0x1443215c0` before processing its shader resources.

The original instance `+0x3a0` traverses overridden MaterialLayersParameters
entries, appending each non-null layer function's textures, then each blend
function's textures via `+0x298`. It then invokes **Parent `+0x3a0`**. Instance
layer references therefore precede parent references. Empty texture overrides
alone would not establish the instance's index mapping.

Direct original reflection-table reads, retained in
`original-texture-instance-layout-1.json`, establish MaterialInstance's
StaticParameters property at `+0x158`, its type getter `0x144802230`, and that
getter's struct block `0x147de9aa0` (directory 34). That struct's reflected
MaterialLayersParameters field is at `+0x30`, matching the writer's instance
`+0x188` array. The original bHasStaticPermutationResource setter writes bit 0
at `+0xd8`; its raw bytes and five instance vtable checks are retained in
`original-texture-instance-additional-slots-1.json`.

The isolated inspector's explicit `--instance` mode retained this source under
`comicbubble-instance-properties-1/`, with both original package hashes. The
serialized StaticParameters has only two StaticSwitchParameters, NoShape and
UseDefaultUV, both overridden true. The MaterialLayersParameters tag is absent;
the original constructor establishes its empty default. Thus no layer texture
is appended before the parent, and the ordered array remains [cloud, tail].

`prove_comicbubble_instance_texture_context.py` requires the pinned parent proof,
the exact parent object/export index, the source package hashes, static
permutation ownership, an absent/empty layer array and no texture overrides. It
also checks the complete original 19-record instance resource and export map.
Its `comicbubble-instance-texture-context-proof-1.json`, SHA-256
`dfe83f55271429ad438fc661de89c029c8a21cfcdaf9828f47920e3e98130487`,
binds the instance's sole regular Texture uniform to index 0/cloud, sampler
source 0, layer/page -1 and virtual false. Preserve the complete two-entry
reference array despite this single compiled texture use. The original instance
resource SHA-256 is
`fab8a4749b2b9b619eb3d4233bdf9b02de19486f1a400e9a578d38e666cbd655`.

The isolated inspector build passes with zero warnings/errors. The instance
replay and five rejection checks pass: layer entries, wrong parent export index,
changed raw-source hash, texture overrides and changed parent proof. These are
offline reference-identity checks; dimensions, conversion and an actual Linux
draw remain unverified.

## TextureProperty dimension access

The next original-code evidence is
`generated/material_pipeline/original-texture-context-evidence-v5.json`, SHA-256
`c70a12b30eea35dfbeae40679050a89231fe520c3a367c79269d808f52aede44`.
It pins 38 immutable artifacts from directories 36–46 and the original
TextureProperty evaluator, and checks eight original vtable entries.

Original Texture2D registration `0x1448046d0`, callback `0x144803e60` and
constructor `0x144602a00` establish class vtable `0x146b3e8e8`. UpdateResource
`0x144615940` calls its `+0x2f8 = 0x144606ba0` resource factory and stores the
result at texture `+0x78`, exactly the pointer read by TextureProperty.

For an ordinary Texture2D, factory constructor `0x144601df0` installs resource
vtable `0x146b3e400` and stores the owner at resource `+0x60`:

- Resource `+0x40 = 0x14460cef0` reads the owner's PlatformData pointer at
  `owner+0xe8` and returns its first uint32, or zero for missing PlatformData.
- Resource `+0x48 = 0x14460cf70` returns the uint32 at PlatformData `+4`, or
  zero for missing PlatformData.

Thus ordinary Texture2D dimensions come from the logical platform header,
without resident-mip scaling. Original Texture2D Serialize `0x144612c90` calls
`0x14462e920`, which obtains the running PlatformData pointer through virtual
`+0x320`, allocates/loads it, and calls `0x14462ece0`. The latter serializes
width and height as the first two 32-bit fields. Do not substitute a decoded
PNG's dimensions without binding them to these source fields.

The factory's virtual-texture branch is different. It installs resource vtable
`0x146b3e458`, stores virtual texture data at resource `+0x68`, and a bias at
`+0x7c`. Size methods `0x14460cf30/0x14460cfb0` return
`max(1, VTData dimension >> bias)` for fields `VTData+8/+0xc`. The branch's
actual Texture2D predicate `+0x310 = 0x14416c3c0` requires flag bit 2 at
`texture+0x65`, non-null PlatformData, and non-null virtual data at
`PlatformData+0x28`. Ordinary and virtual dimensions must not share an assumed
size rule.

TextureProperty `0x14433e4b0` writes only output lanes 0 and 1. Property 0
converts resource width/height to float; property 1 divides the original pinned
`1.0f` constant at `0x1459a86f8` by those dimensions. Missing texture/resource
leaves output untouched; lanes 2 and 3 are untouched even on success. No
caller initialization, actual texture resource existence, or specific asset
dimensions were proven in this step, so it does not justify inserting zero
lanes or enabling numeric flattening.

The scanned texture overlay has one TextureProperty owner:
M_UI_SocialUserItemBorder_Master, vector uniform position 2, property 0, with
a regular Texture child referencing index 0/sampler source 2. Its ordered
reference context, actual source dimensions and the caller's untouched-lane
contract are precise next targets. This step produced no runtime fixture or
Linux draw evidence.

## Reproduction and integrity

All probes ran from the repository root using the existing read-only tool:
`porting/.venv/bin/python -I porting/material_pipeline/view_original_probe.py
--functions <addresses> --out porting/generated/material_pipeline/original-texture-context-N`.
The evidence-bearing function argument sets were: directory 1,
`14433eda0 14433f6c0 14433edd0 14433edf0 14433ef40`; directory 2,
`144342050`; directory 6, `14430b4f0 14431a3b0`; directory 8,
`1443298c0 144328f60 144321030`; directory 10, `144316c60 144786c00`;
directory 11, `14430b550`; directory 12,
`14478b840 14478ba90 14430efc0 14089d900`; directory 15,
`144306a50 1443068b0`. Directory 6's second supplied address belongs to
function `14431a360`, so the output uses that containing-function entry.
Directories 7 and 9 additionally used `--strings 1467b70f0` to collect constructor
references. Directory 14 used `porting/ghidra/decompile_targets.py --only
0x14478da90 --only 0x14478da70 --create-missing --out
porting/generated/material_pipeline/original-texture-context-14 --timeout 30
--max-seconds 180`.

The Ghidra program was never saved or renamed. No original client or rebuilt
UE executable was built or executed; only offline analysis tools ran. Ghidra updated five project
bookkeeping files (`idata/~index.{bak,dat}`, `versioned/~index.{bak,dat}` and
`project.prp`); the individual integrity reports disclose these changes.
The receipt's slot checks and all artifact hashes passed. This establishes
original code mechanics and the bounded parent mapping, not usable texture
materials or visual parity.

The new offline tool and replay commands were:

```sh
dotnet build porting/material_pipeline/original_texture_function_probe/OriginalTextureFunctionProbe.csproj --configuration Release --nologo
dotnet porting/material_pipeline/original_texture_function_probe/bin/Release/net10.0/OriginalTextureFunctionProbe.dll /mnt/vibrato/fortnite-extracted-full porting/generated/material_pipeline/comicbubble-function-properties-1 <seven function package paths from the receipt reports>
python3 -I porting/material_pipeline/prove_comicbubble_texture_context.py --function-reports porting/generated/material_pipeline/comicbubble-function-properties-1 --out porting/generated/material_pipeline/comicbubble-texture-context-proof-4.json
```

Both report tools require a new output destination. Reproduction must choose
fresh destinations to preserve the pinned evidence.

The subsequent instance commands were:

```sh
dotnet build porting/material_pipeline/original_texture_function_probe/OriginalTextureFunctionProbe.csproj --configuration Release --nologo > porting/generated/material_pipeline/original-texture-instance-probe-build-1.log
dotnet porting/material_pipeline/original_texture_function_probe/bin/Release/net10.0/OriginalTextureFunctionProbe.dll /mnt/vibrato/fortnite-extracted-full porting/generated/material_pipeline/comicbubble-instance-properties-1 --instance FortniteGame/Content/Environments/FrontEnd/BackPlates/UITakeovers/M_UI_ComicBubble_Inst
python3 -I porting/material_pipeline/prove_comicbubble_instance_texture_context.py --instance-report porting/generated/material_pipeline/comicbubble-instance-properties-1/FortniteGame_Content_Environments_FrontEnd_BackPlates_UITakeovers_M_UI_ComicBubble_Inst.json --parent-proof porting/generated/material_pipeline/comicbubble-texture-context-proof-4.json --out porting/generated/material_pipeline/comicbubble-instance-texture-context-proof-1.json
```
