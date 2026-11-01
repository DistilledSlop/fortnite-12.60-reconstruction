# Original CommonActivatablePanel native activation events

The original `OnBeginIntro` and `OnBeginOutro` are protected, parameterless
`BlueprintNativeEvent`s with real default behavior. Empty implementations would
lose activation state, visibility changes, Blueprint notifications, delegates and
input subscription changes. This checkpoint proves the state/notification
ordering; the remaining input-object identities below still prevent a complete
faithful panel implementation. It establishes no runtime compatibility.

## Immutable checkpoint and reproduction

`generated/frontend-native-evidence/common-activatable-panel-checkpoint-2/receipt.json`
SHA-256: `43d94146bcbd2186c3af28cbf07d0a98e16c4ab8f22d88b2d4a9e63c8d305fd2`.
It pins 133 immutable input copies, decoded original reflection/vtables, and raw
code windows. Original dump SHA-256 is
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
This is a memory image: **VA = 0x140000000 + file offset**. PE raw section
offsets are inappropriate. Raw windows deliberately do not claim function
boundaries; frozen assembly establishes the actual instruction paths.

Generate a fresh independent checkpoint with:

```sh
python3 -I porting/frontend/inspect_common_activatable_panel.py --output <new-absolute-directory>
```

Decompiler/assembly batches are
`generated/frontend-native-evidence/common-activatable-panel-events-{1..13}`.
The receipt copies these outputs and the exact tools. Batches 3, 9 and 13 used
`ghidra/decompile_targets.py --create-missing --timeout 30 --max-seconds 180`;
the other batches used `material_pipeline/view_original_probe.py --functions`.
Both access the original slim project without saving the program or renaming
symbols. Missing functions were created only in a rolled-back transaction.
Five project bookkeeping files changed; each batch's integrity record discloses
them. The original client was not executed; Ghidra was executed. Initial snapshot
attempt `common-activatable-panel-checkpoint-1` stopped at a failed unwind-table
assertion, before producing a receipt. Checkpoint 2 uses explicitly labelled raw
windows rather than assuming unwind coverage.

## Original ownership, dispatch and state

Panel UHT class block is `145a00360`. `140807980` constructs its UClass metadata;
it is not the C++ instance constructor. GetPrivateStaticClass `1408063f0`
identifies `/Script/CommonUI.CommonActivatablePanel`, size `410`, parent getter
`140805ec0` (`CommonActivatableWidget`, size `2d8`), constructor callback
`140807e00`, and native registration `140809360` (26 entries at `145a038c0`).
The callback calls base constructor `140808180`, then installs primary vtable
`1459edb10`. The base constructor installs `1459ed638`. Secondary vtables begin
at `1459ee010` and `1459edae8`, respectively; do not read base slots beyond its
primary table as extra primary virtual methods. The generated reflection index's
ambiguous Panel vtable candidate is superseded by these actual constructor stores.

OnBeginIntro's function constructor `1408057b0` references FFunctionParams
`147830080`; Outro `1408057e0` references `1478300d0`. Both records have flags
`08080c00`, zero properties, zero parameter structure size and Panel outer.
Native registration binds exec `14080bc70` / `14080bc90`. Each performs
`P_FINISH` then dispatches through the object's actual primary vtable:

| Method | Slot | Panel default body |
| --- | --- | --- |
| OnBeginIntro_Implementation | `4b8` | `1407bb5f0` |
| OnBeginOutro_Implementation | `4b0` | `1407bb630` |
| activation handler | `488` | `1407d1fb0` |
| deactivation handler | `490` | `1407d26e0` |
| intro completion hook | `4d0` | `1406801c0` (`c2 00 00`, return) |
| outro completion hook | `4d8` | `1406801c0` (same return) |

The hooks are virtual and may be overridden. The evidence proves empty *Panel
defaults*, not that hook calls can be omitted. Native exec EndIntro `14080a2c0`
directly jumps to `1407bb5f0`; EndOutro `14080a2e0` to `1407bb630`. Thus the
native event defaults and End methods share the exact machine bodies. This does
not determine whether the original source used forwarding, inlining or folding.

`CommonActivatableWidget.bIsActive` is **exactly byte `288`**: original property
metadata names its Bool setter `140777600`; that setter writes 1 to `[RCX+288]`.
IsActivated exec `14080bae0` reads the same byte. Visibility Bool setters
`140808fc0` / `140808fd0` similarly bind `25d` / `25f` to
`bSetVisibilityOnActivated` / `bSetVisibilityOnDeactivated`. The enum properties
are `ActivatedVisibility=25e`, `DeactivatedVisibility=260`.

Byte `303` is the state returned by IsIntroed (`14080bb80`). It is not a
reflected property in the original Panel block, so `bIsIntroed` is a descriptive
native member name rather than a recovered source identifier.

## Exact defaults and request paths

Intro default calls virtual `4d0` first, regardless of current active state. If
`bIsActive` is false, it sets it true and tail-dispatches `488`; otherwise it
returns. Outro default first clears byte `303`, calls virtual `4d8`, clears
`bIsActive`, and tail-dispatches `490` **unconditionally**, even when already
inactive. The frozen assembly includes every branch/return of these bodies.

ActivateWidget exec `140809a50` reaches `1407a8e60`: only when inactive, dispatch
`4a0`. Base `4a0=1407ce7f0` sets active then calls `488`; base
`4a8=1407ce880` clears active then calls `490`. Panel overrides these request
slots: `1407ce770` returns if byte `303` is already true; otherwise it logs
Activate, sets `303`, and calls the OnBeginIntro event wrapper. Panel
`1407ce810` returns if `303` is false; otherwise it calls `1407ae640`, invokes
OnBeginOutro, and logs Deactivate.

BeginIntro exec `14080a150` reaches `1407ab8f0`, checks the weak pointer at
`3b0` (return ignored), sets `303`, then calls the Intro wrapper. BeginOutro
exec `14080a170` reaches `1407aba20`; only when that weak pointer is invalid,
it calls `1407ae640` then the Outro wrapper. These public Begin methods and
ActivateWidget's guarded request path are distinct.

Wrappers `1408090f0` / `140809130` call FindFunctionChecked `142ec7730`, then
ProcessEvent slot `210` with null parameters. FName globals `1480d2258` /
`1480d2260` are initialized by `1401c5ca0` / `1401c5cc0` from literal
OnBeginIntro / OnBeginOutro. Preserve Blueprint override dispatch.

## Base notification ordering

Base handler `488=1407d2330` returns immediately if inactive. Otherwise:

1. If `bSetVisibilityOnActivated`, call virtual `270` with ActivatedVisibility
   (the original visibility-change log and enum conversion confirm the role).
2. Call BP_OnActivated wrapper `140808b90` using ProcessEvent/null parameters.
3. Broadcast native no-argument multicast at `290` with `140684de0`.
4. Broadcast reflected BP_OnWidgetActivated at `268` with
   `1406a9dc0(delegate,nullptr)`.

Base handler `490=1407d2950` returns immediately if active. Otherwise apply
DeactivatedVisibility when enabled, call BP_OnDeactivated wrapper `140808c10`,
broadcast native no-argument multicast `2a8`, then reflected
BP_OnWidgetDeactivated `278`. Dynamic identities and offsets come directly from
the original class property table. Native delegate source names are unresolved.
`140684de0` iterates bindings in reverse, executes and compacts expired ones;
`1406a9dc0` copies dynamic bindings, resolves object/function and uses
ProcessEvent. Do not replace these with a single Blueprint notification.

## Panel handler ordering and input effects

Panel activation `1407d1fb0` performs the following, even though the embedded
base notification block itself remains guarded by `bIsActive`:

1. Get local-player object through virtual `290=143d5e0b0` (weak field `218`),
   and obtain **CommonInputSubsystem** with `14081b480` / class getter
   `14081de90`, using the player's subsystem collection at `d8`.
2. If the subsystem exists, use `143d5c530` (GetWorld virtual `150`, then world
   field `188`) and `14078c7b0` to obtain **CommonUISubsystem** (getter
   `1408168a0`) via the resulting object's subsystem collection at `d0`.
   If its pointer `50` and that object's pointer `c8` exist, write the result
   of Panel virtual `4c8` to the final object's uint32 `68`. Panel's `4c8`
   is `140680300` (returns zero). The intermediate native types/member names
   are not yet bound; this side effect must be preserved, not guessed.
3. Add a weak UObject-bound delegate to CommonInputSubsystem's multicast at
   `30`, callback `1407c9500`, and store its handle at Panel `3b8`.
4. Execute private single-cast callback `3d0`, passing this Panel.
5. Test weak pointer `3b0`; if valid, resolve it with `142ee5dd0` (no further
   call/store in this body).
6. Execute the complete base activation sequence above when active.
7. Broadcast reflected Panel OnWidgetActivated (`2e0`) with this Panel argument.
8. Execute private single-cast callback `3e0`, passing this Panel.

Panel deactivation `1407d26e0` obtains the same CommonInputSubsystem. If handle
`3b8` is nonzero, it finds the matching delegate handle, unbinds/frees that
entry, compacts the list, and stops. It then calls `1407ae640`, executes private
single-cast callback `3f0`, executes the complete base deactivation sequence when
inactive, broadcasts Panel OnWidgetDeactivated (`2f0`, argument this), then
executes private single-cast callback `400`.

All four private callbacks use `1407bc6e0`, which checks binding/can-execute
and invokes binding virtual `50` with the Panel. They are **single-cast**, not
the reflected multicast properties. Their original source names are unresolved.
The CommonInput callback `1407c9500(this,inputType)` calls Panel virtual `4e0`
(Panel default `1406801c0`, return), then wrapper `1408091b0(this,inputType==1)`.
The wrapper's event identity is the next bounded target.

`1407ae640` acts only while weak object `3b0` is valid. If the 16-byte key at
`3c0` is nonempty and float `308 > 0`, it looks up that key in container `360`
using `1407bdba0` (record stride `98`) and invokes `1407ac590(record+10,0)`
when found. It then resets float `308` and all 16 key bytes and clears two
uint64 fields `a8`/`b0` on the weak object. This exact cleanup is required;
object/action identities and `1407ac590` remain unresolved.

## Implementation boundary / next evidence

The base state, visibility and notification sequence is implementation-ready
against the existing reflected members, retaining native multicast hooks.
The Panel default events can share the real End bodies, but only after the
complete Panel activation/deactivation handlers and private callbacks exist.
The original empty virtual defaults do not justify empty event stubs.

Next targets are `1408091b0` (input callback event), CommonUISubsystem getter/
constructor and field `50` object, its `c8` object/field `68`, writers of weak
field `3b0` and four callback fields, and `1407ac590`/`1407bdba0`. No action-router,
focus-restoration or input-manager source identity is claimed from modern
CommonUI analogies. No Native/Engine/stage file, runtime, UE build or original
client was changed/executed by this investigation.

## Input and callback closure (second checkpoint)

The follow-up supersedes the unresolved identities above without changing the
first receipt or any of its 133 snapshots. New receipt:
`generated/frontend-native-evidence/common-activatable-panel-checkpoint-3/receipt.json`,
SHA-256 `cd36309e30a5df75da88940c59ba452ef9b457d86bfe7f530a3e5f05f3ba27dd`,
217 immutable inputs, including additional batches 14–24. Reproduce into a fresh
directory with the same command plus `--last-batch 24`. The original helper source
used for checkpoint 2 remains available at its immutable source snapshot; the
working helper now also captures subsystem properties and the follow-up windows.

The activation input effect has exact reflected/API identities:

| Original object/member | Binding evidence |
| --- | --- |
| CommonUISubsystem.CommonInputManager, `50` | Original property table `145a0b9c0`; GetInputManager exec `140818410` returns `this+50`; return getter `14080f000` is a thunk to CommonInputManager getter `14080e600` |
| CommonInputManager.GlobalInputHandler, `c8` | Original property table `145a05fd0`; class reference `14080e430` resolves to CommonGlobalInputHandler getter `14080e270` |
| GlobalInputHandlerPriorityFilter, GlobalInputHandler `68` | GetGlobalInputHandlerPriorityFilter exec `14080ffe0` calls `1407c2eb0`, which returns manager `c8` object's `68` or 0 when absent. Setter exec `140811070` calls `1407f66a0`, which writes exactly that field when the handler exists |
| CommonInputManager.CurrentlyHeldActionInputHandler, `a8` | Original reflected interface property at `a8`, two words. These are exactly the words reset by `1407ae640` |

Thus activation sets the **global input handler priority filter** to the result
of Panel virtual `4c8` (Panel default zero). This is a precise name bound through
the original public getter/setter, even though the underlying filter field is
not reflected. No modern action-router identity is needed or implied.

`CommonInputSubsystem`'s native multicast at `30` is separate from its reflected
dynamic OnInputMethodChanged property at `50`; do not subscribe to both or
silently substitute the dynamic delegate. Its bound Panel callback
`1407c9500` invokes virtual `4e0`, then `1408091b0`. The latter wraps
ProcessEvent with one byte. FName initializer `1401c5ce0` binds global
`1480d2268` to literal **OnInputModeChanged** at `145a03378`. Original reflection
names the Bool argument **bUsingGamepad**. The callback supplies `inputType == 1`.

The weak Panel member `3b0` resolves to **CommonInputManager**. Its
StartNextOperation `1407fc000` stores a weak reference to `this` into Panel
`3b0` immediately before either invoking OnBeginIntro or setting bIsActive and
calling activation virtual `488`. This is the same manager whose reflected
Operations array is at `e8` and ActivatablePanelStack at `b8`. Original log
literals name StartNextOperation; this identification does not rely on current
engine source. The function's full 4,061-byte assembly/decompilation is frozen.

StartNextOperation binds private single-cast fields to the following weak manager
callbacks with a captured 16-byte operation identifier. `140786850` constructs
the UObject-weak delegate and copies all 16 captured bytes. It replaces the
single-cast binding rather than appending a multicast listener.

| Panel field | Original callback | Proven consumer effects, excluding logging/temp-string cleanup |
| --- | --- | --- |
| `3d0` | `1407dc080`, log OnPrePanelActivated | If Panel exists, unbind/free `3d0`, then broadcast manager native multicast `60` with Panel |
| `3e0` | `1407dbc80`, log OnPostPanelActivated | If Panel exists, unbind/free `3e0`, then call manager StartNextOperation |
| `3f0` | `1407dc280`, log OnPrePanelDeactivatedForPop | If Panel exists, unbind/free `3f0`. Only if found in manager ActivatablePanelStack: remove every occurrence preserving other order; if Panel byte `304` is true, clear it, call wrapper `1408091f0`, then virtual `4f0`; broadcast manager native multicast `30` with Panel |
| `400` | `1407dbdd0`, log OnPostPanelDeactivatedForPop | If Panel exists, unbind/free `400`, decrement manager outstanding pop count `f8`, and call StartNextOperation only when the decremented count equals zero |
| `400` | `1407dbf20`, log OnPostPanelDeactivatedForPush | If Panel exists, unbind/free `400`; call StartNextOperation regardless of whether Panel exists |

The original unbind operations can occur while the single-cast delegate is being
executed. Preserve delegate execution/lifetime semantics, rather than storing a
raw callable and invalidating it unsafely. The four callbacks are meaningful
operation-completion phases; dropping them leaves the input manager's queue and
stack inconsistent. Manager `30`/`60` multicast source identifiers remain
unreflected/unresolved; the exact offsets, argument and execution semantics are
proven. The names in the table are original log strings, not recovered symbols.

Cleanup `1407ae640` is now more specifically typed. Its `1407bdba0` lookup hashes
the 16-byte key (object pointer plus FName), matching a DataTable row handle,
against entries of stride `98`. When the held duration `308` is positive and an
entry exists, it **broadcasts float 0.0** to the entry's native multicast at
`10`: caller `XORPS XMM1,XMM1`; `1407ac590` preserves XMM1 and supplies it to
each binding. The decompiler's undefined/integer-looking second argument is
incorrect; the assembly proves the float calling convention. It then resets
duration/key and the manager's CurrentlyHeldActionInputHandler interface.
The entry's original C++ type/member name is not recovered; a faithful port must
preserve the float progress broadcast without inventing an action identity.

The remaining implementation dependency is now the **original CommonInputManager
operation queue**, not the two event defaults themselves. `1407fc000` is recovered
in full, including batched-pop paths, binding installation, stack writes,
operation removal and synchronous reentry through completion callbacks. Its
operation record layout (stride `28`) and push/pop enqueue helpers such as
`1407e0c80` still need a reviewed source reconstruction before manager behavior
can be claimed complete. The exact public callback bodies and Panel/Base phase
ordering above can be implemented against typed members; wiring them to empty
manager placeholders would still lose original behavior. Focus restoration and
modern action-router behavior are not additional inferred side effects in these
original bodies. No runtime, build or UI activation claim follows from this
read-only evidence.
