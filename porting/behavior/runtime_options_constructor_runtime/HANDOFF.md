# UFortRuntimeOptions constructor recovery (no body credit)

GPSC `0x142ac77b0` registers `/Script/FortniteGame.FortRuntimeOptions`, size
`0xdf8`, alignment 8, config name `RuntimeOptions`, with internal constructor
`0x142ac95d0`. Its complete thunk is `mov rcx,[rcx]; test rcx,rcx; jne
0x1426eb630; ret`. Thus `0x1426eb630` is the original non-null object
constructor, not the registration callback `0x142acc4e0`. The constructor
starts by calling `0x1444bffb0` and installing primary vptr `0x146226478`.
Its unwind/function extent is `[0x1426eb630,0x1426f3bed)`: 6,458 decoded
instructions, 1,073 call instructions, 24 distinct direct call operands. It
is not a simple collection of scalar default writes. The object initializer,
arrays, strings, allocations, helpers, and ordering must all be recovered; do
not promote a partial constructor or credit it as an implemented body.

An example of *initial stores*, not final CDO values: at `0x1426eb71d` the
constructor stores dword `0x01010001` at object offset `0x80`; at
`0x1426eb72e` it stores word `0x0100` at `0x84`. Strict reflected boolean
setter evidence maps bytes `0x80..0x85`, in order, to
`bEnableSpectatorUpdates`, `bIsTournamentMode`, `bAllowBPTokenRefund`,
`bUseTournamentAnonymousOverrideEnabled`, `bEnableYoutubeLinks`, and
`bEnableGeneratedScreenshotForPortalPreview`; each has mask 1 and `mov8`.
These initial bytes are `01 00 01 01 00 01`, respectively. This does not
prove final post-constructor values: later aliases and callees have not been
closed, and config loading can subsequently modify the CDO. No values were
inferred from frozen CDO, config, or names.

Finite remaining dependencies: decompile and type the constructor with a
complete UFortRuntimeOptions layout, prove all direct and alias-mediated writes
and string/container literals, resolve all 24 call targets including the base
constructor, allocator/growth/copy helpers and repeated map/container helpers,
identify cleanup and any external side effects, and translate all paths in
original order. Reconcile exact reflected bool masks and field types/offsets,
including non-reflected storage; independently establish UE base constructor
and CDO/config initialization semantics. A constructor-splice mechanism must
also be designed by the Native/generator owner. No source proposal is safe
until these close.

Reproduction and evidence:

* `python3 -I porting/behavior/runtime_options_constructor_runtime/inventory.py`
  verifies the entire image-125 SHA-256
  `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`,
  validates function extent and decodes all bytes. It writes a complete
  instruction listing and call histogram to `/tmp/runtime-options-constructor/`.
* `porting/.venv/bin/python -I porting/ghidra/decompile_targets.py --only
  0x142ac77b0:UFortRuntimeOptions.GPSC --out
  porting/generated/decomp/runtime_options_ctor --no-index --create-missing`
  succeeded. The equivalent for `0x1426eb630:UFortRuntimeOptions.Constructor`
  with `--timeout 180` succeeded in 55.3 s, producing 5,189 lines at
  `porting/generated/decomp/runtime_options_ctor/UFortRuntimeOptions/Constructor__1426eb630.c`.
  This raw C still has unknown prototypes, stack artifacts, and untyped fields;
  it is evidence, not a complete typed lift. Ghidra's transactional changes
  are rolled back; project metadata files changed on disk.
* A decompile starting at the tiny `0x142ac95d0` thunk timed out at 30 s; a
  retry was interrupted after disassembly exposed its conditional tail jump.
  It printed JPype shutdown errors on interrupt. The subsequent actual-entry
  extraction above completed successfully.

No Native, manifest, generator, builds, runtime runs, or test changes; current
batch remains build pending. This bounded recovery is intentionally incomplete.

A second, partially named/type-improved probe succeeded in 56.3 s:
`porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input
porting/behavior/runtime_options_constructor_runtime/targets.json --out
porting/generated/callee_probe/runtime_options_constructor --timeout 180`.
Its `0x1426eb630.c`, `.json`, and capped `.asm` are ignored generated evidence;
`index.json` confirms 34,237 bytes, 6,458 instructions, 24 direct callee
identities, zero indirect calls and transaction rollback. The probe's generic
Athena layouts do not type this class; the 203 KB C still uses `undefined8*`
for the object, so it does not close the typed-field requirement. Use the full
SHA-pinned instruction listing, not the probe's 700-line capped asm.
