# Connected CommonUI route source

This directory supplies concrete C++ source and real owner/widget fragments,
using the existing CommonActionBindingData registry. It creates no parallel
binding registry, substitute input manager, abstract callback provider, or
synthetic input preprocessor. Only this directory was edited. Primary owns
Native/generator/Engine integration, builds and commits.

Copy CommonActionRouteTypes.h and CommonActionBindingRoute.{h,cpp} into CommonUI.
The class names and native member names are descriptive; the original runtime
identities are vtables1459ecdd0,1459ecdf8,1459ece30. Source layout is deliberately
not asserted to reproduce shipping binary offsets.

The base collection implements real shared-self construction, always-active
predicate, association, stable removal, bulk removal and global unregister.
WeakSelf represents the original shared-from-this field directly; there is no
second shared-from-this state. Factories install it before initialization. An
expired weak-self association still appends and clears the registry route.
Unregister releases its strong route before deleting the live handle value.

Nodes implement the active-widget/gate predicate, real Slate input processor
registration, parent/child creation and release, activation/deactivation
continuations, inherited configuration and nested optional override, paint-layer
selection, saved focus and exclusive Slate descendant tests. Root source adds
selection changes, activation, config/focus refresh and actual CommonInput method
callbacks. Shared-bound native delegates use the actual existing widget and
input subsystem producers. Children are appended after initialization, matching
1407a9573. Node+a8 is now closed as TArray<TWeakObjectPtr<UWidget>> by1407aa190;
it is represented by RoutedWidgets, not guessed callback storage.

UCommonUIActionRouter.members.inc, bodies.inc, discovery.inc and lifecycle.inc are class/native
body fragments for the real router. They implement the standalone collection,
root/owner/pending-widget arrays, registration1407ed1c0, route lookup/acquisition,
owner fallback creation, processor/routed-widget registration, widget binding
removal, root activation/deactivation/release, and topology drain1407e45c0.
Pending topology uses the original temporary parent-to-children map and root
list, creating modal widgets as roots and ordinary widgets below the first
eligible Slate ancestor. Owner records transfer to real routes, then both pending
arrays reset while retaining capacity. Record.Flag20 is the original byte+d
test selecting the standalone collection during this transfer.

UCommonUserWidget.registration.{members,bodies}.inc implements1407ed3f0 and
the original widget handle array+230. It appends a failed(-1) handle when a router
exists, and does not append when player/router lookup fails. The activatable
bridge owns its NativeDestruct producer: deactivate through the real virtual
request, then call router.NativeRemoveWidgetActionBindings(this) when handles
exist, then Super. This removes associations/pending owner records, not global
records or the widget's handle array.

The router fragments require real includes for UCommonActivatableWidget,
UCommonUserWidget, UCommonInputSubsystem, CommonActivatableWidgetRebuilt,
SubsystemCollection, Engine LocalPlayer/GameInstance/GameViewportClient,
GameFramework PlayerController, SlateApplication/SlateUser, SObjectWidget,
SGameLayerManager and SViewport. Include route types/header before generated.h.
Do not additionally compile RoutePlatformCursorPolicy.cpp: its verified CVar
and helper are incorporated exactly once in CommonActionBindingRoute.cpp.

## Original joins corrected or closed here

* Router virtual+150=1410cf280 is GetWorld, delegating to Outer.GetWorld.
  Controller lookup is LocalPlayer.GetPlayerController(router.GetWorld()).
* 1442f5e70 is LocalPlayer.GetSlateUser(), not a geometry getter. Viewport+f0
  is weak IGameLayerManager; virtual+10 is GetPlayerWidgetHostGeometry(Player).
  Recenter uses SlateUser.SetCursorPosition(Geometry.LocalToAbsolute(local
  size*0.5)), preserving the actual player's viewport geometry.
* 144217cd0 is GameViewportClient.GetUseMouseForTouch.143014960 is Slate
  IsFakingTouchEvents. Static initializer1401c4ee0 registers byte1480d1c81
  under CommonUI.AlwaysShowCursor. The viewport policy call remains eager even
  when Slate's touch flag is true. Evidence/source was supplied by the separate
  route_platform_cursor_policy_runtime component and reviewed before inclusion.
* Widget+2c0 is a zero-argument release multicast. The widget parameter of child
  removal is a bound delegate payload. ReleaseSlateResources calls Super before
  broadcasting, as closed by activatable_route_bridge_runtime.
* Config byte0 joins ECommonInputMode (Menu0/Game1/All2). The+470 nested optional
  payload remains uint8 because its enum identity is unproven. Both optional
  levels are preserved; no invented enum is supplied. Actual base empty virtual
  defaults and desired-focus Blueprint event live in the activatable bridge.

Router input configuration applies state before external effects, mutates the
existing LocalPlayer SlateOperations reply, uses the actual cursor policy and
notifies mode changes even when viewport/controller lookup fails. The target
Engine lacks APlayerController.SetShowMouseCursor; its original1444889f0 only
conditionally logs and writes bShowMouseCursor. The fragment writes that actual
public field. Diagnostic logging is omitted throughout this source component.

## Owner lifecycle and peer integration

Lifecycle source implements complete Initialize1407cdf50, Deinitialize1407b8580
and legacy-manager gate1407c9570. Initialize establishes the CommonInput
subsystem dependency, rebuilt dispatcher, post-GC cleanup, actual virtual input
processor factory, native legacy-manager selection callback, immediate gate from
CommonUISubsystem.CommonInputManager, and Slate OnFocusChanging callback, in that
order. Its Slate processor index is AnalogCursorSettings.PreprocessorPriority:
settings+50 is NOT UIActionProcessingPriority (the latter is native+2c).

The gate writes blocked state first. Legacy selection unregisters the actual
processor, removes the existing FTicker and clears the active root when routing
is enabled. Modern selection registers the processor at its configured priority,
removes the old ticker and installs NativeTickActionRoutes with zero delay.
Deinitialize removes focus callbacks, unregisters the processor, removes the
ticker, clears the active root and empties queued keys in original order. It does
not add extra resets or unsubscribe other weak-UObject delegates absent from the
original. The focus-change callback uses the old focused widget, matching player
index, SetDirectly cause and root-exclusive Slate subtree before saving the
selected node's current focus.

CommonActionBindingData.cleanup fragments extend the existing registry map's
translation unit to compact invalid owners/unbound callbacks after GC. They pin
and disassociate the route before removing the current map iterator entry. The
CommonUISubsystem.legacy.members.inc fragment exposes actual native+58 bool
multicast. Its original producer1407f7b90 emits after real manager creation or
clearing. That already-documented legacy-manager creation feature remains
separate; no manager is fabricated here to satisfy the gate.

NativeTickActionRouteDiscovery() supplies the first part of real ticker1407ffd30:
flush pending owners/widgets, then select a strictly higher painted active root
against the existing root's rank. The complete ticker and virtual+08/+10/+20
input dispatch bodies are supplied by common_action_route_dispatch_runtime.
NativeCreateActionInputPreProcessor is supplied by common_analog_cursor_runtime
and constructs the actual FCommonAnalogCursor. Primary integrates these peer
source bodies with this directory's declarations and performs the final build.
No alternative SlatePostTick callback or pass-through processor is installed.

## Validation

`python3 porting/frontend/common_action_route_connected_runtime/replay.py` PASS:
33 bounded original-image windows,12953bytes, using os.pread and per-window
SHA256. No whole-image read/hash or raw image bytes are stored. All earlier
association/identity/activation/focus evidence is reused from sibling handoffs.
New source has been manually reviewed against target Engine declarations;
primary owns compilation and end-to-end validation after integrating peers.

Preserve the user's planner/orchestrator/reviewer preference. Bounded delegated
work uses Luna medium for small reads, Terra medium/high for regular work, Sol
medium for complex work, fork_turns none. Keep bounded reads/output and rotate
agents at connected component endpoints.

## Generator policy and widget rebuild closure

policy.py supplies apply/declarations/definitions/validate_output/sources and
validate_source for the four real CommonUI classes. Its explicit class contracts
guard block VA, native getter, parent, package and the known construct-owner
functions. Source assembly is owned-only; primary merges dispatch/cursor peer
bodies. registry_cleanup_contract describes the existing canonical Records map,
inside-namespace declaration and fully qualified CPP definition, without writes.

The policy adds CommonUserWidget's private actual TArray<int32> via the standard
model fields merger at original230. Setter1408179d0 proves the reflected
bConsumePointerInput Bool is250; filling that missing Bool offset permits native
storage to be emitted before it in original member order. Do not add a second
handle array from an old declaration fragment. Other native router state is
declared in original30..f0 order. These are source aliases, not an ABI guarantee.

CommonUserWidget.OnWidgetRebuilt1407e03f0 prunes negative or missing registry
handles backward, performs real route acquisition and fresh record association,
then calls UUserWidget.OnWidgetRebuilt. The owner fallback appends the surviving
handle list without deduplication. No invalid handle is silently recreated.
CommonUserWidget.NativeDestruct1407d1e90 removes existing associations/pending
owner records when a typed router exists, then calls UUserWidget.NativeDestruct.
Activatable.NativeDestruct1407d1c00 first deactivates through its virtual request,
removes those associations, then calls UUserWidget.NativeDestruct DIRECTLY,
bypassing the CommonUserWidget override to preserve the original single pass.

Validation: four-class fragment assembly, duplicate-application rejection, native
state declaration order and canonical registry contract passed. Source guards
pass33 bounded windows; primary still owns real promotion and compilation.
