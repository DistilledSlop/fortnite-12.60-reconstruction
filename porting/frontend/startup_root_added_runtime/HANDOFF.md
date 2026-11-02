# Bounded Startup root-added handoff, 2026-11-02

Owned only this directory. No gen/Native/Engine/build/commit changes. Source
candidate exists; it is deliberately unregistered and fails closed on promotion.

New closed joins: Startup inherits Athena (StaticClass145312040 ->1453125f0,
ctor145312900 ->145312b50), so inherited reflected PlayerWidgets+c0 is correct.
The map key/value getters are wrappers142985490 -> FortLocalPlayer14287ccf0 and
1452c41f0 -> FortRootViewportLayout_Athena1452c3f20. Hook145166f50 dispatches
base-root490 before insertion; then derives dormancy/presentation and broadcasts
the actual FortGI native no-parameter multicast5d8. The GI's full constructor
141e48c70 creates that delegate; its declaration is now source-typed.

Gate tags resolved from real native adder145140080/object14839b650:
MatchesTag(UI.State.Athena.Frontend) =>1, exact UI.State.Athena.Replay =>2,
other/invalid =>0. Dormant = !LocalPlayer.IsPrimaryPlayer && presentation!=2.
The source supplies deferred native registration of all four original Athena
tags, rather than fetching invented config tags lazily.

Presentation145152480 is now actual typed Slate behavior: shared wrapper2d8
Collapsed vs SelfHitTestInvisible; native SBox2b8 padding0 vs36; SScaleBox2c8
stretch None vs UserSpecified. Debug literals and Rebuild producers prove the
types. This setter is not opacity/tint. Only2d8 is guarded in the original.

Dormancy14516e780 updates byte2aa only on change and dispatches488. Its log
provides the Dormant name; category identifier remains unjoined. Base488 writes
FortLocalPlayer341=!Dormant, updates visibility, updates actual per-player
UCommonUIActionRouter, and broadcasts bool root multicast2f8. Router producer
1407f78c0 writes99 and, under98==false&&!new99, calls real selected-root1407f4510
with empty shared root. Router ctor initializes98=false,99=true. See existing
common_action_route_activation_runtime/HANDOFF.md92 for selection/deactivation.

Follow-up closure: actual Athena488 and490 are now source-translated in
athena.definitions.inc. Base490 modal/context behavior and proven true-body
1406801d0 are in root-state/context definitions. The original context helper is
144ade110 ->144ade1e0 -> actual FortGlobalUIContext14523f270; semantic predicate
name unknown, constant returntrue bytes independently verified.

Athena488 extends base effects, invokes14516be20, then conditionally acquires
configured Login from the actual modal queue and binds Login native no-argument
2a8 deactivation multicast weakly while capturing Login. The initializer is
empty because its actual target1406801c0 is ret0, not because queue activation is
omitted. Its continuation1451529d0 is also source-translated: remove root's
bindings/queueLogin when nonnull, dormant this root, awaken first game player's
real root. Missing-GI handling preserves the original dereference precondition.

Athena490 retains exact Startup/Athena tag branches, typed config class sources,
numeric legacy selector inputs (unrecovered enum names), root-owned CreateWidget,
old deactivation ->498 clear layers ->assign270 ->attach ->newactivation ->
conditional2a0 reset ->old removal, then notification queue0/Toast handler update.
The source does not unconditionally null CurrentLegacyStateWidget when creation
fails. Current state validity controls fallback to legacy; there is no new class
fallback, fake activation, or diagnostic root launch.

The shared Rebuild/Release worker closed the real cvarproducer. Presentation
now reads MUTABLE GFortUILetoBorderWidth from its RootLayoutSlateSettings.h;
36 was only the captured default, not a constant. Its declarations' logical
widget storage names need joining to this candidate's NativePresentation* names
when integrating, without duplicate native fields or a second tree.

Next exact helper boundary:14516be20 Leto action lifecycle,14516f380 legacy
selector lifecycle (including0),14515b410 root498 clear layers, actual config/soft
class providers and HUD virtual268, modal queue acquisition/activation/removal,
manager/context/root getters, typed state attach/removal, Toast queue setter.
RootStartupProviders.h declares unclosed immediate providers without fabricated
bodies. Full original type records now cover relevant state/data/context/Toast
classes. Parent retains integration/director/manager/CommonUI ownership.

Validation PASS:69 bounded original windows,5 virtual targets,17 complete class
records; fragments load and apply rejects incomplete lifecycle. No build/runtime
claims. git diff --check has no diagnostics (directory untracked at handoff).

Preserve user planner/orchestrator/reviewer preference, bounded task rotation,
economical capped reads, and Luna medium small read-only / Terra medium-high
regular work / Sol medium complex work, always fork_turns none.
