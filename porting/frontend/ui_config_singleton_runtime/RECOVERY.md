# UI configuration singleton

`definitions.inc` supplies `UFortUIDataConfiguration::NativeGet144db86e0`
and its private cached selector. It is an unregistered, unbuilt integration
fragment. It requires the declaration already present in
`startup_root_added_runtime/declarations.json`, plus the usual UE command-line,
config, object-loading, weak-pointer and package headers and
`SingletonDependencies.h`.

The getter checks the weak cache first. On a miss it gets the native
configuration CDO and copies the selected string. Switch cook and BR-only use
Erebus; otherwise BR-only uses BROnly; otherwise Default. It loads an object
whose requested class is UClass, roots the resulting class, and caches its CDO.
On a still-invalid cache it logs, constructs the native config in the transient
package, weakly caches it and roots that object. The loaded class stays rooted
even if its CDO did not produce a valid weak pointer. Neither the static weak
pointer nor the base CDO is a substitute for these lifetimes.

Selector `141ed8a30` caches its first result. For `run=cook`, target platforms
containing Android or iOS force true; otherwise it reads `bBROnly` from
`/Script/FortniteGame.FortGlobals`, then applies `-BROnly` before
`-NoBROnly`. The getter independently tests Switch in the cook target string.
String comparisons and contains use the case-insensitive UE defaults. The
extra loads at `144db8a24..8adc` require both a global byte and `1406801e0`;
the latter is `xor al,al; ret`, so those loads are unreachable in this image.

Two bindings remain deliberately unresolved: `NativeFortGlobalsConfigFilename141ed8a30`
must expose the actual FString global passed as the final GetBool argument
(`1482476f8` RIP target), and `NativeLogMissingUIConfiguration144db86e0`
must reproduce the log category/verbosity guard and the message at
`146d7bce0`. No generic configuration filename or logging category is assumed.
The source also assumes the target UE version's normal `StaticLoadObject`,
`GetDefaultObject`, `NewObject`, and `AddToRoot` APIs match the decoded calls;
integration must compile and verify those signatures. The original includes a
FString construction/destruction around cache initialization at
`140738030/140692260`: the former zeroes16 bytes; the latter frees the first
pointer if nonnull. The fragment's ordinary FString locals supply that lifetime;
this is not an additional unclosed scoped-profiler dependency. The two explicit
config filename and logging bindings still prevent promotion.

Evidence: bounded decode `144db86e0:453`, `141ed8a30:190`, `1406801e0:10`,
`142ee7880:59`, `142ee5dd0:5a`, and targeted UTF-16 reads of the referenced
strings. The `0x40000000` internal object-array bit set in both paths is the
root-set bit. No image-wide scans or hashes were used.
