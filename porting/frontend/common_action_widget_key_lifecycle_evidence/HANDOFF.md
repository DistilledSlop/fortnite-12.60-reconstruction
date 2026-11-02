# CommonActionWidget binding-key lifecycle evidence

Bounded static recovery, 2026-11-02. No production candidate, verified-lifecycle
marker, Native/generator edits, build, runtime, or completion claim. Constructor,
all four identified key writes, native subscription producer, rebuilt/release and
destructor are pinned. The connected receiver still needs authentic refresh and
Slate construction before it can safely supply the existing GetIcon policy marker.

## Key storage and ownership

Original class block `145a01470`, no-register `1408072e0`, constructor thunk `140807de0` ->
`140796260`. Constructor installs vtable `1459ec8b8`, zeroes InputActions and
stores signed int32 -1 at widget +240 (`1407963d9`). This is a borrowed registry
key; no recovered widget path creates or removes its registry record.

`140802150(this, int32 newKey)` returns immediately on equal keys. Otherwise it
proceeds iff `(oldKey >= 0 && Find(oldKey)) || (newKey >= 0 && Find(newKey))`.
If the old key is valid, a negative or missing new key is therefore accepted.
It then performs a fresh Find(oldKey), resolves its weak Owner, calls that
UWidget's GetOwningLocalPlayer (virtual +290), or falls back to this widget's
GetOwningLocalPlayer; it resolves CommonInputSubsystem `14081b480`. When nonnull,
it calls RemoveAll(this) on the subsystem's native multicast at +30. Only after
those calls does `1408021e0` store newKey, then call `1407d0aa0(this, true)`.
Do not cache the registry record/owner across these calls or collapse the two
subscription removals: the second lookup uses the then-current key and owner.

The sole direct call to this setter in the bounded CommonUI range is at
`140802c3c`, in button helper `140802c00`, using button key +b78 and its
InputActionWidget +b68. After setter return it performs a fresh Find(button key),
copies record.Row (+98) into widget.InputActions and calls refresh when found.
This writer establishes how the widget receives a nonnegative borrowed key.

## Both reflected action setters mutate the key

Reflection/native joins prove SetInputAction create `140807910`, exec
`14080c740` -> `1407f6820` (call `14080c7bb`), and SetInputActions create
`140807940`, exec `14080ce80` -> `1407f7710` (call `14080cf37`).

Both first test current key >= 0 and Find(current key). Only when that succeeds,
they repeat owner resolution, remove the widget's old-owner input subscription,
store -1 (`1407f68a6` or `1407f7795`), and call `1407d0aa0(this, true)` before
changing InputActions. A stale nonnegative key that misses Find is retained.
No action equality suppression exists. SetInputAction empties the array, appends
exactly one copied row (even an empty row), then tail-calls refresh `140801d90`.
SetInputActions receives an owning array temporary, copies it into InputActions
unless the array object aliases the destination, calls refresh, then destroys
the incoming array allocation. Its exec makes the owning copy before invoking
the receiver. A C++ by-value array preserves the boundary ownership; a reference
must not accidentally acquire or destroy the caller's allocation.

## Subscription, widget lifecycle, and destruction

`1407d0aa0(this, bool enable)` freshly resolves current key -> weak registry
owner/player or widget/player -> subsystem. Missing subsystem returns. Otherwise
RemoveAll(this) always executes. If enable, it appends a weak UObject native
delegate to subsystem +30, targeting `1407c94c0`; this is AddUObject semantics,
not AddUnique. Native delegate instance weak-owner initialization is
`142ee3b30`; method address is installed at `1407d0b4d`. Callback `1407c94c0`
first invokes real refresh, then broadcasts reflected OnInputMethodChanged
(widget +108) with input type == 1 (Gamepad). Preserve refresh-before-broadcast.
RemoveAll helper `140690620` respects multicast invocation lock +14 and deferred
compaction; production must use the real multicast API rather than a replacement
container or cached delegate handle.

Vtable +2b0 (`1459ecb68`) -> `1407dff40` calls the base no-op, refresh, then
`1407d0aa0(this, true)`; this is UWidget::OnWidgetRebuilt. Vtable +260
(`1459ecb18`) -> `1407ee5e0` is ReleaseSlateResources(bool): reset shared pairs
+270, +260, +250 in that order, resolve the CURRENT key/weak owner/player and
remove this widget's subscription when the subsystem exists, then call base.
It neither resets the rim pair +280 nor clears the key nor removes its record.
Do not move removal ahead of shared-pointer release: release may run destructors.

Vtable +0 -> deleting destructor `1407a7890` -> `14079b5d0`. The latter destroys
cached brush +290, releases rim/progress/icon/box shared pairs +280/+270/+260/+250,
frees InputActions, destroys IconRimBrush +1a8 and ProgressMaterialBrush +118,
frees reflected multicast storage +108 and tail-calls base destructor `14079c200`.
It performs no explicit input RemoveAll and no registry removal or key write.
Its subscription is a weak UObject delegate; explicit removal belongs to the
release hook above, not a newly invented C++ destructor action.

## Exact integration blockers

The key/subscription operations above are source-proven within this bounded task.
Do not set `common_action_binding_key_lifecycle_verified` by merely adding an
initialized member: implement both reflected setters, key setter, native binding,
callback and widget rebuilt/release hooks coherently. All call real refresh at
the specified times. `140801d90` still needs the identity/production configuration
of global gate `14783cb80` read by `14081d130`, plus actual Slate graph construction
`1407e6fc0` and owning shared members +250..+288 / cached brush +290 with their
proper release/destruction lifecycle. Existing scratch handoffs provide mappings,
but no complete production implementation has been reviewed here. These are
concrete receiver blockers; inventing refresh or retaining a permanently -1 key
would change behavior. Module/settings work belongs to root's separate task.

Next fresh bounded recovery should start with `14081d130:10` and data
`14783cb80:4`, then recover the writer/registration of that exact global using
targeted existing CommonInput initializer/xref evidence. It must establish the
real console/config identity and default/producer before implementing the gate.
Refresh replay window is `140801d90:1c9`, owner helper `1407c3ab0:55`, first-row
info selector `1407c20f0` (bound the function before extending its read), and the
already mapped Slate graph `1407e6fc0:cd0`. Existing static Slate mappings are in
`/mnt/vibrato/fortnite-work/common-action-widget-slate-mapping-1/HANDOFF.md`;
consult those before further recovery. Direct Slate API windows are
`142fd0d50:29`, `142fd17e0:d9`, `142fc6030:1b0`; complete release window is
`1407ee5e0:154`. Preserve the unguarded progress/box dereferences after the icon
pointer check and repeated SetVisibility calls documented in refresh evidence.

Ownership implications for production: reuse `CommonActionBindingData::Find`
and the record's existing weak Owner; do not add a second registry or strong
UPROPERTY owner. Create's owner contract is UWidget. The widget stores a borrowed
int32 and owns its array, three brushes and four Slate shared pairs through the
normal UE C++ member lifecycle. Use existing subsystem
`NativeInputMethodChanged.RemoveAll(this)` and `.AddUObject(...)` (no delegate
handle or AddUnique substitution); resolve subsystem afresh in each specified
phase. No source evidence authorizes deleting a registry entry from widget
destruction, or migrating subscriptions on arbitrary owner changes outside the
recovered hooks. SetInputActions is a required producer even if an immediate
button caller only uses the single-action API.

## Replay and search scope

Run `python3 porting/frontend/common_action_widget_key_lifecycle_evidence/replay.py`.
PASS checks both setter reflection records/native joins and 19 bounded SHA-256
windows via pread (including ctor/thunk, both execs/receivers, key setter, binding,
callback, rebuilt/release/destructors, RemoveAll, button caller and vtable slots).
It does not hash the whole image or execute the original. Image and historical
whole-image identity are in original-contract.json. Disassembly command:
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA:SIZE`.

Targeted existing handoffs supplied constructor/vtable identity and known setters.
Then a bounded `pread(0x90000, 0x795000)` inspected the original CommonUI code
neighborhood [140795000,140825000) for direct rel32 calls to the three known
mutators and decoded +240 stores with Capstone. It discovered SetInputActions
and the rebuilt hook. The other +240 dword store `1407eb7cb` writes an object
returned by `140791790` in a Slate construction function, not this widget; other
matches are qword/byte fields of other constructors or stack stores. This is a
bounded discovery aid, not a whole-program proof against arbitrary indirect or
external writes. No scan of the whole image or untrusted IDA files occurred.

Root owns integration and builds. No commits; owned files are only this directory.
Collaboration preference persists: bounded delegates, fork_turns none; Luna medium
read-only, Terra medium/high regular, Sol medium complex; economical excerpts.
