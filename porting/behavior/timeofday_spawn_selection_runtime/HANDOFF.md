# Integrated time-of-day spawn and refresh

NativeSpawnTimeOfDayManager (0x141fbd1f0) and NativeRefreshTimeOfDayManager
(0x141fd72d0) are integrated with the shared tracked resolver, STW data
provider, context predicate, theme cache/selection, and Athena override caller.
The evidence windows are pinned in native-evidence.json. Probes remain under
porting/generated/callee_probe/aircraft_larger_recovery{,2}.

The source uses UWorld::bIsWorldInitialized, AlwaysSpawn and RF_Transient;
retains the iterator through selection/spawn; and preserves mission, zone,
world, default fallback order and repeated default-object checks. A raw
single-pointer subclass copy preserves the native wrong-base CDO checks before
the final spawn-class validation. The shipping visibility callback is empty.
The asm return is R14, initially null: failed selection returns null even if
the stored manager field remains nonnull. Refresh does not clear that field
before destruction or the nested spawn operation.

No tests/build/runtime were run. Broader runtime and parity gates remain open.
