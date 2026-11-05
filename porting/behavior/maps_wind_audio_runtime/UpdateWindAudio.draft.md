# UpdateWindAudio 0x141de18c0 — typed reconstruction, incomplete

Source-only analysis. This is deliberately not a compilable implementation: the named
`Effect_*` operations below are unresolved native effects, not callable stubs. Do not
integrate it or count it as runtime coverage. Offsets and line references below refer
to `porting/generated/maps-wind-ready-dispatch/_adhoc/0x141de18c0__141de18c0.c`.
`DeltaSeconds` is the second argument; reflected field names come from
`AFortWindManager.h` and `FFortWindResponder.h`.

```cpp
// Conceptual member body; Effect_* denotes a required, unresolved native effect.
if (GetNetMode() == NM_DedicatedServer) return;
UWorld* World = GetWorld();
if (!World) return;
ABuildingSMActor* const PreviousActor = ActiveAudioWindResponder.WindUpdatingBuildingSMActor;

if (/* *0x14812a5f8 (Fort.Wind) == 0 */ Effect_GlobalWindGateIsZero()) {
    Effect_AssignDefaultResponder_141dbf770(ActiveAudioWindResponder);
} else {
    const FVector Viewer = Effect_ResolveViewerPosition(World); // default globals if unavailable
    Effect_RefreshNearbyAudioCandidates_141ddf5c0(*this, Viewer);
    if (bAllowResponderAudio) {
        float BestDistanceSquared = AudioWindMaxResponderDistance * AudioWindMaxResponderDistance;
        FFortWindResponder* Best = nullptr;
        for (int32 Index : NearbyCacheIndicesForAudio) {
            if (!WindResponders.IsValidIndex(Index)) continue;
            FFortWindResponder& Candidate = WindResponders[Index];
            if (!Candidate.WindUpdatingBuildingSMActor || !Candidate.WindAudio ||
                !(Candidate.WindSpeed > 0.0f)) continue;
            // NativeBounds_0x68 contains min XYZ then max XYZ. Compute per-axis
            // squared distance outside this box, zero for coordinates within it.
            const float DistanceSquared = Effect_ExactDistanceSquaredToNativeBounds(Viewer, Candidate);
            if (DistanceSquared < BestDistanceSquared) {
                BestDistanceSquared = DistanceSquared;
                Best = &Candidate;
            }
        }
        if (Best) {
            if (Best->WindUpdatingBuildingSMActor != ActiveAudioWindResponder.WindUpdatingBuildingSMActor) {
                ActiveAudioWindResponder = *Best; // native memberwise/deep copy, incl. non-reflected tail
                if (!Effect_MapHasNonNullComponent(WindActorToWindResponderAudioMap,
                                                   ActiveAudioWindResponder.WindUpdatingBuildingSMActor)) {
                    Effect_AcquireInitializeAndInsertAudioComponent(*this, ActiveAudioWindResponder);
                }
            }
            // Best found, even if the same actor: skip default assignment.
        } else {
            Effect_AssignDefaultResponder_141dbf770(ActiveAudioWindResponder);
        }
    }
    // Disabled audio retains the previous active responder; it is not defaulted.
}

ABuildingSMActor* const CurrentActor = ActiveAudioWindResponder.WindUpdatingBuildingSMActor;
if (PreviousActor != CurrentActor) {
    if (Effect_MapHasNonNullComponent(WindActorToWindResponderAudioMap, PreviousActor))
        Effect_142b53d90(this, /* mapped component for PreviousActor */);
    if (Effect_MapHasNonNullComponent(WindActorToWindResponderAudioMap, CurrentActor))
        Effect_142b53d50(this, /* mapped component for CurrentActor */);
}
for (/* each occupied map entry, with native iterator removal semantics */) {
    ABuildingSMActor* Actor = /* key */;
    UAudioComponent* Component = /* value */;
    if (!Actor || !Component) { /* remove current entry */; continue; }
    float Volume = /* Component float at +0x230 */;
    if (Actor == ActiveAudioWindResponder.WindUpdatingBuildingSMActor) {
        Volume += DeltaSeconds * AudioWindInterpSpeed;
        Effect_1440c9db0(Component, AudioWindSpeedParameterName, ActiveAudioWindResponder.WindSpeed);
    } else {
        Volume -= DeltaSeconds * AudioWindInterpSpeed;
    }
    if (Volume <= 0.0f) {
        Effect_StopAndMaybeReturnToPool(*this, Component);
        /* remove current entry */;
    } else {
        if (Volume >= 1.0f) Volume = 1.0f;
        if (Volume != /* Component float at +0x230 */)
            Effect_1440cc390(Component, Volume);
    }
}
```

The distance helper is written as an effect to avoid assuming the byte array's
alignment or silently treating its seventh byte group as an `FBox` validity flag.
The actual three-axis comparison is explicit in source lines 189–205: for each
coordinate compare minimum, then maximum, and add squared difference when outside;
only the first six floats are read. The comparison against the maximum is strict,
so equal-distance candidates do not replace an earlier one, including equality at
the initial radius. The `Volume <= 0` notation above is equivalent for finite
values only: the native code tests `< 0`, then `< 1` and `== 0` separately. Keep
that exact ordering when implementing, including for NaNs.

## Unresolved native effects and evidence

* Lines 83–94: `143f7a920` is `GetNetMode` and 1 is dedicated server; virtual +0x150 is `GetWorld`. The global gate points to Fort.Wind.
* Lines 102–134 and 347–389: zero global gate or no eligible responder calls `141dbf770` with a zero-initialized responder and destroys its temporary nested arrays. Determine the resulting default responder values before substituting a reset.
* Lines 136–155: `14468f240` first weak controller, class check `142a3d9f0`, controller +0x2b0 and virtual +0x708 supply the viewer vector; fallback globals `148218630/638` are zero FVector (see `maps_wind_make_runtime/defaults-evidence.json`). Prove controller class and view target/position API before translating. Candidate-refresh details follow below.
* Lines 212–274: responder assignment is fieldwise with deep TArray copies (`1406af1d0`, `141dbc1c0`, `140e88080`) and raw bounds/tail. The emitted struct preserves the native FBox bytes, speed, effective distance and setup byte; its normal copy semantics match these fields.
* Lines 275–342: on actor change with no non-null mapped component, pop pool, reject invalid pooled object via object flags or allocate via `1446cc8e0`/`142ed7360` with outer actor and flags 0x100040; pooled object is renamed/reoutered via virtual +0x138. `144125240` receives actor +0x130; `143f72cc0` fills a position; subtract actor root +0x1d0 location or fallback globals and call `144123220`. Set component +0x1f0 to responder sound, +0x230 to zero, call `1440c2bb0`, then virtual +0x540 and conditionally +0x530, and insert map entry through `141dbc760`. Resolve class, attachment/transform, registration, activation and ownership semantics before coding.
* Lines 390–409: conditional `142b53d90` for old actor and `142b53d50` for new actor dispatch events via `142ec7730` and virtual +0x210 (ProcessEvent). Each wrapper accepts a second 8-byte argument and passes its address as event parameters. The caller decompilation drops that argument. Root assembly at 141de2132 loads the old map entry value into RDX before call141de213e; 141de217d loads the new map entry value into RDX before call141de2189. Both arguments are the corresponding mapped audio component. Reflection lists `OnWindAudioBecameActive` and `OnWindAudioBecameInactive`, each taking `WindAudioComp` at offset 0, consistent with these transitions; mapping the wrappers to those event names requires the name globals or other direct evidence. Presence means lookup succeeds and mapped component is non-null, not simply that a key exists.
* Lines 410–546: native sparse-map iteration; null key/value removes entry without stopping or pooling. For current actor, `1440c9db0` runs before volume handling with parameter name and responder speed. Nonpositive volume calls virtual +0x538, then if object is not flagged in global object array calls `1440d4a00`, virtual +0x400 with zero-initialized argument, `143f814f0(manager, component)`, virtual +0x138 reoutering to manager with flags 0x15, and pushes to pool. Removal occurs even if object flagged. `1440cc390` is called only on a changed clamped volume. Resolve component APIs, validity check, cleanup order and map removal semantics.

Integration requires the complete audio effects and dependencies above, plus the
separately unfinished initialization and sampling paths. `TickActor` wrapper
0x141dde360 invokes sampling 0x141ddfad0, this audio helper, then
`AActor::TickActor`; preserve this ordering only once both helpers are complete.
No build, test, generator, manifest, or native integration was performed.

## Candidate refresh 0x141ddf5c0

Capture: `porting/generated/maps-wind-audio-spatial-deps/_adhoc/0x141ddf5c0__141ddf5c0.c`. If either nearby cache has entries, compare squared distance from the last refresh position at manager +0x30c to Viewer against `ViewerMovementDistanceForRefresh²`; strictly less returns without refreshing. Otherwise clear both caches. When both caches are empty this movement check and clearing are skipped. If spatial tree pointer +0x2e0 is null, return without updating the stored viewer.

With a tree, store Viewer; query `141dbcf90` using an XY rectangle centered on Viewer with half-width `UpdateWindMaxResponderDistance` and validity byte 1. For each returned index, access the responder without a bounds check and require a non-null actor. The update-cache threshold is `min(global_14463f630()->float_at_0x18 * NativeEffectiveDistance_0x88, UpdateWindMaxResponderDistance) + ViewerMovementDistanceForRefresh`; append if squared 3D distance from the responder bounds center to Viewer is strictly less than the threshold squared. If the update cache is nonempty, set `NextNearbyIndexToUpdate` to `min((int)(rand() * 3.051851e-05f * Count), Count - 1)`. Preserve the native call and arithmetic semantics when resolving the RNG.

Only if `bAllowResponderAudio`, query the same tree with XY half-width `AudioWindMaxResponderDistance`; append returned indices whose responder actor is non-null and whose 3D bounds-center distance squared is strictly less than `(AudioWindMaxResponderDistance + ViewerMovementDistanceForRefresh)²`. It does not filter sound or wind speed at this stage. Both queries reuse a static scratch array and clear its count before each query. Tree query `141dbcf90`, tree construction/maintenance, `14463f630` and its +0x18 scale, RNG and concurrency assumptions remain dependencies; do not replace the query with an unproven scan.
