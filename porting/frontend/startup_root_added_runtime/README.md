# Startup root-added source boundary

This unregistered candidate implements Startup hook `145166f50`, real root
dormancy setter `14516e780`, real root presentation setter `145152480`, the
base dormancy-change body `145166040`, and router gate producer `1407f78c0`.
It also supplies Athena virtual488=`1451660f0` and490=`145167a40`, base490,
and the weak Login-widget deactivation continuation1451529d0.
It is not a normal-startup success claim. No generated source, Engine source,
build, runtime or generator policy registration is changed. `apply()` rejects
promotion until the actual remaining virtual/delegate lifecycle is supplied.

The installed Startup manager table `147011440+298` calls this hook. Its order
is: conditional base-root cast and virtual490 state propagation; insert into
PlayerWidgets; compute presentation from the current tag; set root dormancy;
set presentation; obtain the typed FortGI Outer; broadcast its native no-argument
multicast. There is no post-creation null-result guard. The source keeps real
virtual dispatch; it does not replace root490 or488 with an empty body.

The tag computation is exact: invalid tag yields0; hierarchical
`MatchesTag(UI.State.Athena.Frontend)` yields1; otherwise exact equality with
`UI.State.Athena.Replay` yields2; other valid tags yield0. Dormancy is
`!LocalPlayer->IsPrimaryPlayer() && Presentation != 2`.

## Closed metadata and native producers

Startup genuinely derives from Athena. `145312040+84` loads base StaticClass
getter `1453125f0`; Startup callback `145312900` calls Athena constructor
`145312b50`, then replaces its vtable with147011440. Athena constructor creates
its c0 map. This joins the inherited reflected `PlayerWidgets` atc0 to the actual
Startup hook. Reflection types its key as LocalPlayer via wrapper142985490 ->
14287ccf0, and its value as FortRootViewportLayout_Athena via wrapper1452c41f0 ->
1452c3f20. Its base root class is1452c5f60. The hook's incoming UUserWidget root
has the Athena LayoutClass invariant; the source static downcast adds no new
runtime check. The original first cast is to the base root before virtual490.
Do not add a second checked cast or a null early return.

The tag-adder object14839b650 has vtable146e431f8 ->145140080. It constructs
prefix `UI.State.Athena` and registers Frontend/BattleRoyale/Spectator/Replay
with native comment `(Native)`, storing at object8/10/18/20. The two consumed
global tags are14839b658 and14839b670. `StartupRootNativeTags.h` reconstructs
this deferred native-adder registration. Its object must be instantiated at
module startup; a first-use lazy object after gameplay-tag finalization is
incorrect. Do not substitute equality-only matching for Frontend.

Root constructor1452c69a0 initializes native bool2aa false, shared widgets
2b8/2c8/2d8 empty, and multicast2f8 (default array and compaction threshold2).
The setter log proves2aa is dormancy: `%s RootViewportLayout Dormancy changed
for [%d] from [%s] to [%s]`. It compares before changing, writes2aa, then calls
virtual488. It computes the owning player's controller id/primary status before
the logging level check. The log's exact category identifier is still unjoined;
`LogFortUI` is a candidate source name, not a recovered original identifier.

Presentation widgets come from actual RebuildWidget145169f10:

| Root native storage | Concrete producer/type | Presentation2 | Other values |
| --- | --- | --- | --- |
| 2d8/2e0 | wrapper shared widget, assigned14516a4a5 onward | Collapsed | SelfHitTestInvisible |
| 2b8/2c0 | SBox, constructed14516a781..834; debug literal1459ecee0 | padding0 | mutable Leto border width |
| 2c8/2d0 | SScaleBox, constructed14516a68f..74e; literal1459f7328 | EStretch::None | EStretch::UserSpecified |

`145152480` guards only the wrapper pointer. It does not independently guard
the box or scale box. The branch constants are visibility byte2 at147c522f1,
visibility byte17 at147c522f4 and stretch0/7.147e1b7d4 is the mutable
FortUI.LetoBorderWidth backing float, captured as36. The source reads shared
GFortUILetoBorderWidth from root_layout_slate_runtime/RootLayoutSlateSettings.h;
it must not freeze the captured default as a constant.
1430ffcf0 is SBox::SetPadding (FMargin), not a color setter.143100980 is
SScaleBox::SetStretch: it updates its enum attribute and refreshes safe-zone
scale. Local UE4260 SBox.cpp68..76 and SScaleBox.cpp345..353 support these joins.
This does not supply or replace the entire RebuildWidget method.

GI class getter1428ea7f0 points to constructor callback1428eb510 ->141e48c70.
The constructor initializes multicast5d8 with zero storage at5d8/5e0 and2 at5e8.
Broadcast140684de0 invokes each callable+50 with no user argument and performs
the real multicast lock/compaction behavior. `DECLARE_MULTICAST_DELEGATE` is
the correct native zero-parameter signature, not a dynamic reflected delegate.

The base root's virtual488 body145166040 writes FortLocalPlayer native341 to
inverse dormancy, changes root visibility, updates the local player's actual
CommonUIActionRouter, then broadcasts bool multicast2f8. The typed router lookup
is14078c710, reached through GetOwningLocalPlayer by1407c1720. It does not select
CommonInputSubsystem. Root lookup can return null, and this consumer guards it.
Router1407f78c0 always writes native byte99. If98 is false and the new99 is false,
it invokes the real selected-root setter1407f4510 with an empty shared root.
Router constructor140797b40 writes word98=0100:98=false,99=true. Names for those
unreflected bools are aliases. This is a real gate producer; the selected-root
operation must retain its deactivation and delegate lifecycle, already bounded
in `common_action_route_activation_runtime/HANDOFF.md`.

## Remaining exact boundary

Athena root installs table146fc80c8, where488=1451660f0 and490=145167a40;
both override bodies are now supplied in athena.definitions.inc.
The base table146fc7bf0 has488=145166040 and490=1451679d0. Base-only dormancy
handling remains insufficient. Base490 gets the actual FortGlobalUIContext,
then applies the disconnected-controller modal visibility/activation behavior.
Predicate1406801d0 is independently proven original bytes `b0 01 c3` (true);
its semantic source name remains unknown. The source preserves its real body,
rather than guessing connectivity or input-device conditions.

Athena488 executes base effects, calls real Leto update14516be20, then only for
an awake nonprimary FortLocalPlayer failing14238f060 and manager-state exactly
Startup.Login loads StartupStateWidget_Login. Its real modal queue acquires and
activates the Login widget, then root weakly binds Login native multicast2a8
(no parameters; common_activatable_panel_native_events.md125) while capturing
the Login pointer. The queue initializer's actual original target1406801c0 is
ret0; the supplied empty initializer reproduces that specific original body,
not the queue or root lifecycle. Acquisition failure calls the same continuation
with null. The continuation removes this root's callbacks and removes Login
from the queue when nonnull, sets this root dormant, and sets the first game
player's real root awake. It uses this widget's actual GameInstance and retains
the original missing-GI dereference precondition.

Athena490 performs modal/base handling, Leto update, then requires primary
player or14238f060. Startup.Login/SubgameSelect and Athena.Frontend/HUD/Replay
branches use the exact reflected soft classes and expected native class checks.
The HUD class remains real data-config virtual268. Legacy numeric selectors are
kept as numeric inputs to14516f380 because their source enum identities are
unrecovered. No guessed enum names are supplied. A valid state without an
acceptable new-state class invokes that legacy setter and returns. Otherwise
it clears legacy via14516f380(0), creates the state widget with this root as
owner and NAME_None, deactivates the previous state, dispatches real498 clear
layers, assigns CurrentStateWidget, attaches, activates, conditionally clears
CurrentLegacyStateWidget, and removes the previous widget. Finally it gets
NotificationManager's queue0 (UINotificationQueues+28) and assigns it to the
ToastDisplayArea_ToastHandler's queue through144fb3e40. It does not replace
the queue assignment with an arbitrary notification or a null fallback.

The remaining immediate helper boundary is explicit in RootStartupProviders.h
and contract.json:14516be20,14516f380,14515b410, actual config/typed soft-class
providers, queue add/activate/remove and native delegate producers, context and
manager/root getters, state attach/remove and Toast queue side effects.
Root Rebuild/Release and cvar source now exists in root_layout_slate_runtime;
integration must share its real storage rather than creating a second tree.

Integrate the logical native fields within the full class constructors and
GC-visible reflected storage; declarations.json is not a raw original-offset
layout patch. The source uses normal typed objects, widgets and native delegates.
No raw-offset execution or alternate widget tree is supplied.

Verification: policy.validate_original checks bounded byte windows and installed
slots; validate_classes checks complete original reflected records; fragments
returns reviewable source inputs. These checks establish provenance and type
joins, not compilation, widget creation or normal frontend runtime.
