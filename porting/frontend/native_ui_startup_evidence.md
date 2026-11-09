# Original native UI startup

The original game instance initializes UI through **FortUIDirectorSubsystemBase**,
which selects/creates a FortUIManager and manages a root layout for each local
player. It does not directly create a generic root widget. The actual cooked
startup manager selects RootLayout_Athena through its LayoutClass property and
inherits the native nonlegacy-input default. This checkpoint proves that path
through CreateWidget and AddToPlayerScreen; state/layer/context initialization
and complete startup gates still require the next original-code closure.
No native implementation or interactive-UI result is claimed.

## Immutable evidence and reproduction

Receipt `generated/frontend-native-evidence/native-ui-startup-checkpoint-2/receipt.json`,
SHA-256 `4738065312af979ccc72165502a702c5d7fb94938ce1b41afb78b06fd2590427`,
pins 152 immutable inputs, original raw code/table windows and decoded metadata.
It includes the complete decompiles/assembly from `native-ui-startup-{1..9}`,
the narrowly bounded CUE probe source/binaries/dependencies, both raw source
packages and their complete export/property reports. Original memory-image
SHA-256 is `604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
VA is **0x140000000 + file offset**; stale PE raw offsets are not used.

```sh
dotnet build porting/frontend/native_ui_startup_probe/NativeUiStartupProbe.csproj -c Release --nologo
dotnet porting/frontend/native_ui_startup_probe/bin/Release/net10.0/NativeUiStartupProbe.dll <full-loose-extraction-root> <fresh-report-directory>
python3 -I porting/frontend/inspect_native_ui_startup.py --packages <fresh-report-directory> --output <fresh-checkpoint-directory>
```

The isolated probe reads existing CUE dependencies and builds only its own
binary. It never rebuilds/overwrites shared Cue4Inspect or writes to an asset.
`native-ui-startup-packages-2` contains the reports used here. Package run 1 is
preserved; run 2 adds the original name map for independent tagged-property
framing. Checkpoint attempt 1 stopped before a receipt because the guard expected
SoftClassProperty; the original serialized tag is **SoftObjectProperty**. The
corrected guard proves the exact original tag/value/terminator to export EOF.

Ghidra was opened exclusively through the existing read-only probe. The program
was never saved and symbols never renamed; each batch discloses five changed
project bookkeeping files. No original client, UE build/runtime or staged
application was run or changed. All four product milestones remain false.

## Game-instance entry and director ownership

`141e6c120` is the 961-byte InitializeUI body, identified by its original literal
`UFortGameInstance::InitializeUI()`. Its gates are preserved:

- Return when the world helper `144217e60(this)` yields null.
- Return for game-instance byte `8b0` values 3/2/4, respectively identified by
  original logs as KairosCapture, KairosInitial and KairosLoadElectraPlayer.
- Return when startup-phase helper `141ed7750` returns a byte below 3.
- Return when the current-map comparison matches a Kairos map. The exact
  configured comparison providers remain to be typed; do not replace this with
  a guessed path test.

After these gates it calls `141856620(this)`, obtaining a game-instance subsystem
through collection `d0`, requested class getter `142b26df0`. That getter's
original literal is **FortUIDirectorSubsystemBase**. It then iterates GI local
players (`38`, count `40`), calling **director** virtual `290` then `280` for
each player. These offsets are on the director, not on FortUIManager.

Observed direct callers include `141e72150` (OnPostLoadMap, supported by its
original log), `141e664e0` (updates mode byte `8b0` from `8b1` before UI init),
and `144da09b0` (engine-side world/GI path). Their complete bodies/xrefs are
frozen, but registration and complete lifecycle ownership of these callers are
not yet bound. Startup-phase helper `141ed7750` reads a separate global owner
through `141df8a70` and class `1427ba7e0`; it is not a guessed GI flag.

FortUIDirector getter `1453113f0` names `/Script/FortniteUI.FortUIDirector`,
size `c8`, Game config, parent FortUIDirectorSubsystemBase. Its constructor
callback `145312730` installs primary vtable `14700fc30`. Relevant exact slots:

| Director slot | Original body | Effect |
| --- | --- | --- |
| `268` | `14515a550` | Initialize/load manager classes and subscribe lifecycle callbacks |
| `270` | `14514afb0` | Set current manager null and remove subscriptions |
| `280` | `145146b80` | Update/select manager, then add this local player's root when possible |
| `290` | `14514b080` | Remove this local player's previous root |

Original property metadata binds Entryway/Athena/CampaignManagerClassPath to
`48/60/78`, their loaded class pointers to `90/98/a0`, CurrentManager to `a8`.
Correction: the earlier Campaign/Entryway ordering here was reversed; reflection
block14700d9a0 and director_player_root_runtime pin the named fields and selection.
Initialize resolves the three configured soft paths and binds callback
`145174cf0` through `142ba9030` (handle `b0`) plus callback `1451540c0` to the
multicast returned by `142ec9f90`. Their full event identities remain open.

## Source class providers and nonlegacy choice

The immutable extraction's DefaultGame.ini section
`[/Script/FortniteUI.FortUIDirector]` sets:

| Property | Original value |
| --- | --- |
| EntrywayManagerClassPath | `/Game/UI/Foundation/UIManager_Startup.UIManager_Startup_C` |
| AthenaManagerClassPath | `/Game/UI/Foundation/UIManager_Athena.UIManager_Athena_C` |
| CampaignManagerClassPath | `/Game/UI/Foundation/UIManager_Campaign.UIManager_Campaign_C` |

The receipt binds the full original config SHA and a safe extraction of these
three section-owned lines; it does not copy unrelated config or print secrets.

UIManager_Startup's original class export has SuperStruct FortUIManager_Startup
and **ClassWithin FortUIDirector**. Its CDO export is exactly 45 bytes: one
LayoutClass/SoftObjectProperty tag (payload 12 bytes), value
`/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C`, empty subpath, then
None and exact export EOF. Independent framing checks all FName numbers, tag
size/index, property-GUID flag and terminator against the original name map.
There is **no bUseLegacyInputSystem tagged override**. Native base constructor
`145143bd0` sets that Bool false; the actual Startup constructor chain does not
change it. Thus this specific cooked startup CDO inherits false, not just a
parser's guessed default.

RootLayout_Athena_C has SuperStruct FortRootViewportLayout_Athena. Its class
record uses the original template asset
`/Game/UI/Foundation/RootLayout_Athena.WidgetArchetype`; the probe preserves all
659 nonmetadata exports. Neither this class record nor its tree has been cooked,
resized or replaced by this investigation.

## Manager selection, construction and player root

Director `280` first invokes `145176990`. This validates/reloads all three
manager class pointers, uses its game instance/first local player and source
subgame helpers to select one, compares that class with CurrentManager's class,
and calls `1451746d0` when different and GI byte `7bb` permits. The exact helper
semantics and source enum identities in selection are the next target; an
implementation must not simply always select Startup.

Set-manager `1451746d0` captures currently attached player entries from the old
manager (entry flag `+10`), captures an interface state, calls old manager
virtual `270`, marks it for GC, creates the desired validated FortUIManager
class with **director Outer**, assigns CurrentManager and calls its virtual
`268`. It re-adds captured players, restores interface state when permitted,
then broadcasts director native multicast `30`. NewObject is subject to the
original class/Within ownership; game instance is not the manager's direct Outer.

Startup's actual constructor-installed table is `147011440`; `268` is
`14515aac0`. That calls base `14515a790`, which applies
bUseLegacyInputSystem to CommonUISubsystem, then sets the FortUIManagerInterface
object/interface pair at `110/118`. It does not construct the root widget there.

With nonnull local player and manager, director add calls `145146bc0`:
update UI state through `1451755b0`, subscribe a weak manager-bound callback to
the player's native multicast `348`, then search RootViewportLayouts. If an
entry exists, attach it and set its `+10` flag true. Otherwise call `14514ab50`:

1. Obtain manager world via virtual `150`; resolve this **local player's**
   controller in that world with `14447f220`. If unavailable, return without
   creating a widget. The helper iterates world's controllers and compares
   each controller's LocalPlayer against the requested player; its null-world
   path uses the player's controller member.
2. Resolve manager LayoutClass (`28`) synchronously using `141a67470(...,true)`.
3. CreateWidget helper `14078a2a0` calls the UserWidget creation path
   `143d52a00` with that controller, class and NAME_None. It validates a
   UserWidget result. Preserve the original owner and class.
4. Append RootViewportLayouts entry of stride `18`: LocalPlayer at 0, RootLayout
   at 8, attached flag at `10`. The public reflected struct has the first two
   fields; the original native flag is additional state.
5. Attach using `145146470`: assign a LocalPlayer context with `143d8eba0`,
   call **AddToPlayerScreen(ZOrder=0)** (`143d44530`), then manager virtual `298`.
   AddToPlayerScreen checks owning player and dispatches the screen-add virtual;
   it is not interchangeable with an ownerless AddToViewport call.

Before a replacement, director `290` unbinds the manager from player multicast
`348`, removes the player's RootViewportLayouts entry, removes its Slate widget
from the screen through `14516cba0`, invokes `145174fa0` on the root, and manager
virtual `2a8`. Startup also calls base state-change propagation `1451657f0` via
its `290` override: each applicable root receives virtual `490` with current
state tag (8-byte underlying name), followed by manager native multicast `78`. State providers, root
`490`, layer/widget creation and context initialization are not yet fully typed.

## Next exact closure

The source-bound creation path is concrete enough to reject arbitrary root
creation and forced legacy input. Next recover source startup-phase/subgame
selection, director callback identities, the native manager `298/2a8` root
hooks, root `490` state transition and native construction/layer registration.
Then join that path to BlueprintContext registry creation and owning-local-player
context insertion. Current source evidence cannot claim those behaviors are
implemented, that a widget was instantiated, or that an interactive root exists.

## Login state, native Slate layer and phase follow-up

Receipt `generated/frontend-native-evidence/native-ui-startup-checkpoint-3/receipt.json`,
SHA-256 `dbedd66b0726d7dfe9917a049be2adfd31cc01de7faf6c3bc70a543d6b2eb663`,
adds complete batches 10–26, all four source packages, the expanded probe's new
immutable binaries/source, original state tags, root constructor-installed tables,
UI-data metadata and startup-phase enum. Checkpoint 2 and its 152 inputs remain
unchanged. The current probe exports UIManager_Startup, RootLayout_Athena,
DefaultUIDataConfiguration and LoginScreen; use a full extraction root. Exact
checkpoint-2 replay uses its own frozen two-package probe, not a changed working
binary. The shared CUE tool was not rebuilt.

The first expanded probe requested BROnlyUIDataConfiguration as well and failed
closed because that asset is absent in the supplied full Windows extraction.
Its partial `native-ui-startup-packages-3` reports remain preserved; they are not
a complete success receipt. The bounded tool now explicitly selects either the
Windows-default UI data or `--br-only` (which still fails when that required
asset is absent). Successful `native-ui-startup-packages-4` exports all four
Windows-default packages, including 1,668 nonmetadata LoginScreen exports.
No absent class was replaced with a different asset.

### Exact hierarchical startup state tags

State is **FGameplayTag**. `143e93680` contains original literal
`STAT_FGameplayTag_MatchesTag` and checks both own/parent tag entries; an
equality-only FName implementation would change behavior.

Static constructor `1405ff6a0` registers native tag-adder object `14839b838`
and installs one-entry vtable `146e430e0` → `14513f920`. That method constructs
the `UI.State.Startup` prefix plus the child strings Login/SubgameSelect/Rejoin,
registers each native gameplay tag, and stores results at object `8/10/18`:

| Original global | Exact tag |
| --- | --- |
| `14839b840` | `UI.State.Startup.Login` |
| `14839b848` | `UI.State.Startup.SubgameSelect` |
| `14839b850` | `UI.State.Startup.Rejoin` |
| `14839b900` | `UI.State.Startup` (separate deferred callback `145140c80`) |

This mapping follows stores in the native adder, not import order or a guessed
match to the observed log. Manager Startup slot `280=14514b4b0` requires an
AFortGameState in its local-player context; `1442f3c10` obtains the context
world's GameState (`world+130`), and the original class getter `142908ec0`
names FortGameState. If that cast fails the provider returns an empty tag.
Consequently a generic Entry world's class is not evidence of a valid startup
state provider.

The Startup provider uses the local player's native flag `340`, predicate
`14238f060`, two global predicates `141f3ad70/141f3ade0`, and the game instance's
**RejoinCheck**. Original property metadata binds RejoinCheck to `GI+440`;
getter `141e62e50` returns that exact field. Rejoin predicate `142261610` tests
RejoinCheck byte `28==1` and byte `121==false`. The full provider preserves the
Rejoin continuation condition, otherwise selects Login or SubgameSelect. The
remaining login/global predicate identities must be recovered before replacing
them with named high-level decisions; do not force Login unconditionally.

### Data-driven LoginScreen class and replacement order

Getter `144db86e0` identifies itself as `UFortUIDataConfiguration::Get` and
loads the configured class, caches its CDO weakly, with the original error/
default-object path when class load fails. Original metadata binds:

| Original offset | Typed property |
| --- | --- |
| `30/40/50` | Default/BROnly/ErebusUIDataConfigurationPath |
| `310` | StartupStateWidget_Login |
| `338` | StartupStateWidget_SubgameSelect |

The original DefaultGame.ini chooses
`/Game/UI/Foundation/Config/DefaultUIDataConfiguration.DefaultUIDataConfiguration_C`.
That asset's CDO sets **StartupStateWidget_Login** to
`/Game/UI/Foundation/StateLayouts/LoginScreen.LoginScreen_C`. The native root's
Login branch accesses precisely data `310`, synchronously resolves it through
`145141970`, and validates the Login native class `1453167f0`. Class identity
is not a substituted widget name.

Selector `141ed8a30` reads `/Script/FortniteGame.FortGlobals:bBROnly`, honors
`-BROnly`/`-NoBROnly`, caches its result, and has original cook-platform branches.
The UI-data getter additionally distinguishes Switch cook selection. Source
config and code are preserved; no claim is made that the missing BR-only asset
would successfully load or that all platform choices have been ported.

Athena root constructor callback `1452c6390` calls base constructor
`1452c69a0`, then installs **146fc80c8**. The reflection index candidate
146fc7bf0 is the base table. On the actual Athena table, state-change slot `490`
is **145167a40** (1,731 bytes). After initial disconnected-controller modal and
other guarded work, the startup branch uses hierarchical MatchesTag and the
source UI-data class above. When a new state widget is selected, its order is:

1. Clear the legacy state selector through `14516f380(root,0)`.
2. CreateWidget with the **root widget as owner**, validate
   FortUIStateWidgetBase, and retain the previous state pointer.
3. Deactivate the previous state widget if present.
4. Call virtual `498=14515b410` (NativeClearLayers). That body deactivates the
   disconnected-controller modal, clears modal queue, confirmation queue and
   confirmation/error state through the exact original helpers.
5. Assign reflected **CurrentStateWidget at 270**, then attach the new widget
   through `145174c00`.
6. Activate the new widget via `1407a8e60` when nonnull, reset native field `2a0`,
   then remove the previous widget through `145174fa0`.
7. Perform the final conditional controller/confirmation-widget update.

The modal at **328** in the early state-change block is reflected
ProgressModal_ControllerDisconnected. It is not CurrentStateWidget. The new
pointer is stored at `270`; CurrentLegacyStateWidget is a separate pointer at
`2a0`. Those names come from original property records, not byte-offset analogy.

Native root RebuildWidget is **145169f10**, constructor-table slot `2a8` on
both base and Athena; ReleaseSlateResources is `14516c7c0` at `260`. Rebuild
contains original source-file/line literals for FortRootViewportLayout.cpp,
constructs native state-content **SBox** (`2e8/2f0`) and **SOverlay**, and wraps
the inherited UserWidget rebuild result (`143d801e0`). Optional nonprimary-player
Leto/scaling branches are also recovered in full and must remain guarded.
`145174c00` uses the state's TakeWidget result and changes the SBox content;
null selects the original null Slate widget. `1430fe6b0` is the short content
setter. ReleaseSlateResources calls its super and releases all four native
shared widget pointers. The native Slate layer is therefore required alongside
the original WidgetArchetype/WidgetTree; merely adding a state widget to an
invented canvas does not reproduce this path.

### Startup readiness comes from FortAssetManager

The original DefaultEngine.ini selects
`AssetManagerClassName=/Script/FortniteGame.FortAssetManager`. Phase getter
`141ed7750` calls `141df8a70`, which reads GEngine's asset manager and validates
FortAssetManager (`1427ba7e0`), then `141dfb870` reads its native byte `6cd`.
Absent/wrong asset-manager class returns phase zero.

The original enum constructor `142b24950` binds block `147bd80e8`, named
EFortStartupPhase. Its values are InitializingEngine=0, EarlyStartupLoading=1,
EarlyStartupFinished=2, **GameStartupLoading=3**, GameStartupFinished=4,
Invalid=5 and Count=5. Thus the UI gate permits GameStartupLoading onward; it
does not authorize setting an invented ready bit. Actual asset-manager phase
progression is an unresolved prerequisite for faithfully invoking InitializeUI.

### Context registry boundary

Fresh `140822100` establishes one process-global shared BlueprintContext manager
at `1480d3380/3388`, created lazily with the first supplied owner and shared-from-
this support. Later calls return that same manager; it is not a per-GI factory.
This refines the existing context-routing notes. Context class/player insertion,
deferred world callbacks and teardown still need to be bound before implementing
GetContext for the Root/Login Blueprint scripts. The receipt proves source
creation/state/layer prerequisites, not interactive widgets, authentication or
context behavior in the rebuilt client.

## Checkpoint 4: source phase transitions and context lifecycle

New immutable receipt
`generated/frontend-native-evidence/native-ui-startup-checkpoint-4/receipt.json`,
SHA-256 **3459ba0ebe7777b8d60c6015a290c8c4d820c5b63afe1a7d849a681cdaee9834**,
freezes 374 inputs, including the updated inspector and original batches 1–39.
The previous receipts and their content-addressed source snapshots remain
unchanged. Reproduce with the inspector's `--last-batch 39 --packages
porting/generated/frontend-native-evidence/native-ui-startup-packages-4` and a
fresh `--output`. This checkpoint advances the unresolved boundaries described
above; it does not establish a complete startup implementation.

### Phase changes require original feature states

`141e17270` is the original phase updater. Its state lookup `141df80d0` searches
an asset-manager map at `6d0`, keyed by a byte, with `1a8`-byte entries; the
state byte is at entry `+8`, and a missing entry returns zero. The recovered
phase transitions are:

| Guard, in original priority order | Effect |
| --- | --- |
| Current phase is 4 | Return immediately |
| State for key 1 or key 6 is exactly 8 | If current phase <5, set phase4, conditionally enable automatic audio precache, then call `141dedeb0(true)`; invalid backward transition logs |
| Otherwise state for key1 or key6 is >=5 | If current phase <4 and not3, set phase3 and conditionally call `141dedeb0(false)`; already3 returns |
| Otherwise current phase0 | Set phase1 and disable automatic audio precache |
| Then current phase1 and state for key3 >=5 | Set phase2 and call `141dedeb0(true)` |

The table intentionally retains numeric feature keys until their original map
key type and enum binding are proven. `142b24410` is an original enum accessor
through `147bd8078`; it is not by itself proof of the map's key identity.
`141df4af0` contains the original FinishInitialLoading log and only performs
its game-startup block at phase3: superclass work, bundle operations, a feature
state operation `141dec440(this,6,8,0)`, then iteration over FortLocalPlayers
calling `14238a5c0`. The exact readiness/event chain and those helpers still
need recovery before replacing the current FortAssetManager stub.

A second real phase gate now ties contexts to player insertion. Original
`141e4f680` calls base `144205160` and returns if it fails. It then invokes
BlueprintContext module slot `40` to create this player's contexts **only if
`141ed7750()>3`**, before assigning a primary player if needed and calling
UIDirector slot `280`. The original `AddLocalPlayer: Set %s to Primary Player`
literal and full 532-byte body are retained in batch38/39. Its module name comes
from static initializer `1402c0cd0`, with the literal BlueprintContext and
FName destination `14812df80`. Context creation for players already present
when the phase finishes remains an explicit caller gap; do not bypass it by
unconditionally initializing contexts in a LocalPlayer subsystem.

### Actual registry construction and destruction

The module factory `140822a00` allocates an eight-byte module object and installs
`145a0f798`. That vtable's custom API is `40=14081f190` create,
`48=14081f2c0` destroy, and `50=14081f3f0` lookup. Create/destroy contain
original BlueprintContext player/GI log messages. Both use the process-global
shared manager; lookup retains its null-player guard.

Manager creation `140820790` prunes expired weak player keys, finds or inserts
the player's map, obtains all classes derived from BlueprintContextBase, and
skips abstract classes. It creates every context with **the LocalPlayer as
Outer**, inserts it under its actual class key, and only after the complete
class set exists iterates the map to call each context's virtual `280()`.
This ordering permits peer-context lookup during initialization. The map's
iteration order must not be replaced with an invented sorted order.

The original BlueprintContextBase registration passes Within getter
`1408143c0`, which resolves to original LocalPlayer getter `144786700`.
Constructor callback `1408234d0` calls the USubsystem constructor and installs
`145a0f930`; its source class contract therefore needs **Within=LocalPlayer**.
The installed base table contains:

| Slot | Target / base behavior |
| --- | --- |
| `268`, `270`, `280`, `288`, `298`, `2a0` | Original shared RET body `1406801c0` |
| `278`, `290` | `1408229f0`: load primary vtable and tail-jump to slot `288` |

These are slot identities, not guessed C++ method names. In particular, world
initialization at `290` may reach a derived override of `288`; the base RET does
not justify dropping that phase. The full raw forwarding body has no extra
state change or call.

Destroy `140820f00` obtains the LocalPlayer's world, finds its weak-key map,
calls every context's virtual `2a0(context,world)`, marks each context with
original GC flag `20000000`, and removes the player entry. The exact outer
caller of the module's destroy API still needs binding.

### Deferred world notifications and GC ownership

The manager constructor `14081fca0` binds original globals `147ce0788` and
`147ce07b8`. Their original producer bodies are frozen in batch29:
`144691ab0` broadcasts the former after world initialization with World and
initialization values; `144687a40` logs UWorld::CleanupWorld and broadcasts the
latter with World and both cleanup booleans. Current Engine delegate names
OnPostWorldInitialization/OnWorldCleanup agree with those signatures.

Initialization callback `140822a30` stores a weak World under its name and,
when the manager's weak GI is valid, schedules `140822910` using the original
GI TimerManager (`+c0`) and `144627d90` (original STAT_SetTimerForNextTick).
The deferred callback removes the pending name entry, resolves its weak World,
then uses `144640340` to locate its engine world context. It only dispatches
when that context's owning GI (`+218`) equals the manager's GI. Iteration
`140821ca0` visits player/class maps through copied class maps, and callback
`1408206a0` calls each context's virtual `290(context,world)`.
Cleanup `140822c00` applies the same GI check and uses `1408206c0` to call
virtual `298(context,world)`; its extra cleanup booleans do not affect that body.

The manager is also an original FGCObject referencer, registered in its
constructor through the original UGCObjectReferencer path. Treating all context
storage as weak references would lose this ownership. Destructor `1408200c0`
removes both delegate handles (`c8/d0`), destroys maps, and unregisters that
referencer. The full ownership traversal and process-global shutdown caller
remain implementation prerequisites.

Batches27–39 used only read-only view/decompilation helpers, except missing
leaf definitions in batches31/34, which were created in transactions and rolled
back. Ghidra was not saved or renamed; its five bookkeeping-file changes are
retained in each integrity record. The original client was not executed.
No Native/Engine/stage source, runtime cache, or reconstructed lifecycle changed.

## Checkpoint 5: existing-player startup and real LocalPlayer teardown

Separate immutable receipt
`generated/frontend-native-evidence/native-ui-startup-checkpoint-5/receipt.json`,
SHA-256 **40cc5232ca791d8d635e6e0d11a35d7deb1eefb86f31627d2c8714f34f8d8736**,
freezes 924 inputs and batches1–48. Reproduce with `--last-batch 48`, the same
packages-4 selection, and a fresh output directory. Checkpoint4 remains
unchanged. Batch40 includes the original shared module cache's consumers;
retaining these full bodies avoids reducing unrelated context consumers to
inferred call patterns.

The original FinishInitialLoading body `141df4af0` calls **14238a5c0** for
each FortLocalPlayer after `141dec440(this,6,8,0)`. This 108-byte helper first
invokes the proven BlueprintContext module slot `40` with that LocalPlayer.
Then, if the LocalPlayer's field `288` is null, it tail-calls LocalPlayer
virtual **280**. This closes checkpoint4's *location* gap for context creation
of players present before phase4; it does not justify calling the helper
outside its original startup sequence.

That virtual is not a guessed subsystem initialization hook. The original
FortLocalPlayer class getter `142989f60` registers constructor callback
`142988640`, which calls actual constructor **142366a90**. That constructor
calls LocalPlayer constructor `1442eb960`, then installs **14612ead8** as the
primary table. The base constructor installs **14642c9d0**. Their relevant
slots are:

| Slot | Base LocalPlayer | Original FortLocalPlayer |
| --- | --- | --- |
| `278` | `1442fa540` | `1423a2410` |
| `280` | `1406801c0` (RET) | `14238bde0` |
| `288` | `1442fa570` | `1423a2490` |

The base `278` stores the viewport client, calls virtual SetControllerId,
and initializes the subsystem collection. Base `288` deinitializes that
collection. These bodies agree with public Engine LocalPlayer's ordered
`PlayerAdded`, `InitOnlineSession`, `PlayerRemoved` API; the source-installed
vtable and superclass calls establish the actual override binding.

**FortOnlineAccount is the reflected field at LocalPlayer offset `288`**,
with original Object metadata (decimal648), rather than an input manager.
Original InitOnlineSession `14238bde0` returns while startup phase<4.
When the account is absent it creates original class **FortOnlineAccount**
(getter `142a07490`, literal `/Script/FortniteGame.FortOnlineAccount`) with
the LocalPlayer as Outer, stores it, and calls account virtual `260` with
this player's world. The full 1,198-byte method retains further stored callback
forwarding and GI/service subscriptions; these are not optional or ported by
this investigation. They require their own typed method/callback identities
before implementing the entire method.

Original PlayerRemoved **1423a2490** (319 bytes) performs its preceding service
unsubscribe/cleanup, calls base `1442fa570`, then, if FortOnlineAccount is
present, invokes its virtual `270` and clears the field. Finally it tail-calls
BlueprintContext module **48** with this LocalPlayer. This binds context
destruction to the actual original PlayerRemoved override. It would lose
source behavior to replace that entire method with only DestroyContexts.

### Strong value traversal is recovered

The context manager's installed FGCObject table `145a0fbd8` has its second
entry **140820460**. That method iterates each live player entry and calls
**14081f5d0** on its nested class map. The nested helper traverses each live
24-byte key/value entry and passes the **value pointer by reference**
(entry+8) to the collector's virtual `38`, with the original null referencing
object/property arguments. It does not trace the weak-player key or class-map
key through that call. This is concrete strong ownership of context instances;
retain the original nested traversal and weak-key pruning distinction.

### Remaining startup boundary

`141dedeb0` is now named by its own original log:
**SetStartupWindowClosedForAlwaysLoadedAssets**. It obtains a shared manager,
locks its critical section at `1a0`, updates byte `1c8` if different (or logs
an already-set value), then unlocks. It is not an implicit UI-ready event.
The phase state writer `141dec440` writes the selected entry's state, resets
associated state, calls phase updater `141e17270`, then retains its original
notifications and dependent-feature work. The full body is frozen; none of
those phases are replaced with a ready flag.

Original enum accessor `142b24410` directly binds block `147bd8078`, whose name
is **EFortGameFeatureState**. Its raw enumerators are Unknown0, Unavailable1,
Downloading2, DownloadFailed3, Available4, Preloading5, Preloaded6, Loading7,
Loaded8, Count8. This pins the original state enum separately from the still
unproven map-key type. The accessor's first pointer `142a02c30` obtains the
FortniteGame package; it is not a separate enum constructor.

Next exact dependencies are the source feature-loading/state notification
chain that invokes InitializeUI, the FortOnlineAccount/service callbacks in
InitOnlineSession/PlayerRemoved, and the actual derived context initialization
bodies used by Root/Login. Registry creation, world dispatch, GC ownership,
phase-finish creation and removal are source-proven prerequisites. They do not
establish that existing generated context bodies provide original behavior.
No Native/Engine source or runtime changed. Batch42's missing constructor
callback definition was created only in a rolled-back Ghidra transaction;
all other new batches used the read-only view helper. Program save/rename did
not occur, and the five bookkeeping-file changes remain disclosed.

## Checkpoint 6: Root/Login use concrete tickable LocalPlayerSubsystems

Separate immutable receipt
`generated/frontend-native-evidence/native-ui-startup-checkpoint-6/receipt.json`,
SHA-256 **33135dc68bec913acc8e96076576ca693c0f6b838eee64d524aeae840c4856bc**,
freezes 1,036 inputs, batches1–63, original hierarchy/hook windows and typed
property metadata. Reproduce with `--last-batch 63`, packages-4, and a fresh
output. The old receipts and frozen source snapshots remain unchanged.

### Closed routing and inheritance boundary

The original LoginScreen report contains typed GetContext return properties
for **FortFrontEndContext** and **FortGlobalUIContext**; RootLayout_Athena
contains FortGlobalUIContext. Original getters `145235090` and `14523f270`
register these classes with superclass thunk `1428cd110`, resolving to getter
**14298a020** and the literal **FortLocalPlayerSubsystem**. Consequently these
requests take the already-proven **LocalPlayerSubsystem collection branch**
in `140822490`. The legacy BlueprintContextBase registry is a different family
and cannot supply their original initialization.

FortLocalPlayerSubsystem registers size **30**, callback **142988660**.
That callback calls USubsystem constructor `144602970`, installs primary
**1463afca8**, and returns. It does **not** construct a secondary tickable base.
Its ShouldCreate slot `260` returns true; Initialize `268` and Deinitialize
`270` are RET. The concrete context constructors introduce the second base:

| Class | Constructor callback | Primary table | Secondary table, at object+30 |
| --- | --- | --- | --- |
| FortFrontEndContext | `145236e60` | `146d4c470` | `146d4c6e8` |
| FortGlobalUIContext | `1452422f0` | `146f3bce8` | `146f3bf60` |

Each calls the original tickable constructor **144617dd0(this+30)**, then
stores its concrete secondary table there. Their primary tables end after
`270`: reading primary+278 as another primary virtual would accidentally read
the adjacent secondary table. Add tickable inheritance on the two concrete
classes, not FortLocalPlayerSubsystem.

The tickable constructor installs **146b42428**, registers its address in the
shared tick array (or deferred queue under the original initialization guard),
and excludes tick type2. Its destructor `144619bb0` removes that registration.
The eight-method secondary layout and constructor algorithm agree with the
pinned public Engine FTickableGameObject sources; naming that C++ interface is
supported by this correspondence, while the original tables control behavior.

| Secondary slot | Base tickable | FrontEnd | GlobalUI |
| --- | --- | --- | --- |
| `0` Tick | `_purecall` | `144cec950` | `144cecf50` |
| `8` tick type | return0 | return0 | `142992770` |
| `10` IsTickable | true | `144cd7d10` | true |
| `18` GetStatId | `_purecall` | `1407c5180` | `1407c5180` |
| `20` destructor | `144619bb0` | `144cbec84` | `1452429f4` |
| `28`, `30` paused/editor | false | false | false |
| `38` GetWorld | null | `142992760` | `142992760` |

The `_purecall` identity is primary import evidence: target **1458092e1**
indirect-jumps through IAT **1458fd5b8**, whose original import descriptor/lookup
names **VCRUNTIME140.dll!_purecall**. Both Tick and GetStatId in the base point
there; they are not empty defaults. Base zero/false/true leaves are separately
frozen. `142992770` returns `1 + ((UObjectFlags >>4)&1)` from the complete object,
so the ordinary instance uses type1 and the CDO type2; the pinned Engine enum
names those Always/Never. FrontEnd uses conditional type0 and its nontrivial
213-byte IsTickable guard. `142992760` adjusts secondary-this by `-30`, then
tail-calls the primary object's GetWorld at `150`.

**One exact unresolved body prevents claiming complete tickable closure:**
shared concrete GetStatId **1407c5180** jumps to **1407c5138**, then to
**1495a0c8c**. At that destination this pinned memory snapshot contains
invalid/protected instruction bytes (`39 9a fb d6 76 2e c5 fa ...`), and Ghidra
reports bad instruction data. Its actual stat-ID behavior is unavailable here.
Do not invent a zero stat ID, use `_purecall`, or call it an empty method.
The next exact source target is the decoded body behind that gateway from
independently authorized original evidence. The concrete tick bodies are
available (FrontEnd1,534 bytes, GlobalUI893 bytes) and have real per-frame work;
adding inheritance with no-op ticks would omit it.

### Derived initialization dependency map

Original primary hook bodies are:

| Class | Initialize `268` | Deinitialize `270` |
| --- | --- | --- |
| FrontEnd | `144cd59b0`, 403 bytes | `144cc3570`, 123 bytes |
| GlobalUI | `144cd5b50`, 2,266 bytes | `144cc35f0`, 717 bytes |

FrontEnd Initialize performs this order:

1. Validate its Outer as FortLocalPlayer and, if valid, weak-bind callbacks to
   that player's native delegates `348`, `360`, `378` (targets `144cda450`,
   `144cdc3f0`, `1442a3540`).
2. Call **14460fe10(Collection, FortGlobalUIContext::StaticClass())**. Its
   original collection guards/creation behavior match InitializeDependency;
   preserve the dependency rather than inserting an unrelated global singleton.
3. Call **144cbf970**, the full 4,965-byte feature-configuration helper.
4. NewObject original class **FortServerBrowserHelper** (`1452c5c50`) with
   this context as Outer, assigning reflected **ServerBrowserHelper at400**.

FrontEnd Deinitialize resets eleven consecutive feature records, clearing
owned dynamic storage. The captured body does not perform an added blanket
unsubscribe. Its player `378` callback **1442a3540** is a closed 14-byte body
broadcasting reflected **OnPlayerLoggedOut at198**. The `360` callback
**144cdc3f0** first broadcasts **OnPlayerLoggedIn at188**, then performs guarded
daily-notification effects; a broadcast-only replacement would discard them.
The `348` callback and feature helper retain additional controller/service
subscriptions and need typed callback closure before the whole method is ported.

GlobalUI Initialize parses original `subgame=` and `tournamentmode` options,
binds LocalPlayer delegates `348`, `3d8`, `360`, `378`, then binds GI/service,
platform and input/settings notifications. Its helpers **144cb7480** and
**141e63650** bind the GI and GI field448 respectively; full implementations
and callback addresses are retained. GlobalUI Deinitialize removes the exact
recorded subscriptions, clears its source timer **AddictionMsgTimer at398**
through the world's timer manager, and releases associated state. Its recorded
LP RemoveAll calls cover348/3d8/360; do not silently add378 as a cleanup fix.
A complete port still needs those private delegate and service identities.

The useful implementation seam is the complete original GetContext routing
onto native engine subsystem collections plus the separately proven registry
for its actual consumers. The Root/Login classes must retain their own native
initialization, ticking and teardown; registry insertion or init-only behavior
cannot substitute for these methods.

### Actual UI-entry caller remains separate

New original callers of InitializeUI are retained in batch49. The body at
**141e72150** contains the original OnPostLoadMap literal and calls InitializeUI
after decrementing GI pending counter688 to zero, outside its guarded world
mode. **141e664e0** names itself HandlePendingActiveGameFeature and calls it on
entering the Kairos feature range from outside that range. **144da09b0** validates
GEngine/GameEngine and its GI before calling it. The actual live registration
of the first and third bodies remains unbound; missing static references are
not proof that they are unreachable.

The installed FortGI primary table **145fef1a0** has Init `280=141e67d50`,
which jumps through obfuscated prologue gateway **1492b711a**, reaches
`141e67d8a`, then branches to `141e67d44`. Existing Ghidra function splits do not
recover that complete control-flow graph. This is an explicit next target,
not permission to invoke InitializeUI unconditionally or force startup phase4.

The two failed batch53 command forms are preserved in a separate frozen
failure note; the successful command uses repeated `--only` arguments. One
failed form stopped at argparse, the other raised AddressFormatException after
opening Ghidra without creating a function; that exception did not emit a
bookkeeping diff. Successful missing-function probes53/61/62 used rollback
transactions. No program was saved or renamed. No original execution,
Native/Engine/stage mutation or runtime parity is claimed.

### Director lifecycle source follow-up

The lifecycle sources can be typed more narrowly from the director callbacks and
source implementations. `142c976d0` supplies the core ticker; `142ba9030`
registers the zero-delay delegate. Callback `145174cf0` takes a float delta,
updates manager selection, invokes current manager virtual `278` with that
delta if present, and returns true. The handle at director `b0` is removed in
Deinitialize. `142ec9f90` supplies the post-garbage-collection multicast:
`142e520b0` broadcasts it at the end of collection, corresponding to UE 4.25
`FCoreUObjectDelegates::GetPostGarbageCollect()`. Callback `1451540c0` scans
weak widget entries at `b8`, diagnoses surviving widgets with `OBJ REFS`, and
clears the array. Deinitialize removes subscriptions owned by the director.
Probe excerpts are in `porting/generated/bench/ui_integrate/lifecycle-evidence/`.

The manager transition still requires the original interface methods and their
argument types. It saves the old interface state and attached players, shuts
down the old manager, creates the selected class with director outer, initializes
it, restores the players and interface state, and broadcasts the director
multicast even if the result is null. The lifecycle discovery does not close
that transition or the game-instance startup caller.
