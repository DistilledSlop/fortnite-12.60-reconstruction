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

Original property metadata binds Campaign/Athena/EntrywayManagerClassPath to
`48/60/78`, their loaded class pointers to `90/98/a0`, CurrentManager to `a8`.
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
