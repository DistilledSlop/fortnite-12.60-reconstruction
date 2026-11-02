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

The filename global `1482476f8` is `GGameIni`: at `142c76288` it is the
output argument to `142c77790`, with the literal `Game` at `1459a4c58`.
Adjacent calls initialize `Input`, `Compat`, and `Lightmass`. This matches the
local engine's `FConfigCacheIni::LoadGlobalIniFile(GGameIni, TEXT("Game"))`
in `Core/Private/Misc/ConfigCacheIni.cpp` and the declaration in
`Core/Public/CoreGlobals.h`.

The logging category is `LogFortUI`: static initializer `1405dcde0` constructs
an FName from the literal at `146d716a0`, and passes category object
`14838e2e0` to `142c475f0`, with default verbosity 5 and compile-time
verbosity 7. At `144db8955` its verbosity byte must be at least 2 before
logging with the category FName at `14838e2e4` and verbosity 2 (Error).
`UE_LOG` supplies the corresponding guard and exact message at `146d7bce0`.
`SharedFortUILog.h` and `.cpp` supply its single shared declaration and
definition for FortniteUI; integration must emit them once and use that
declaration for all consumers.
The source also assumes the target UE version's normal `StaticLoadObject`,
`GetDefaultObject`, `NewObject`, and `AddToRoot` APIs match the decoded calls;
integration must compile and verify those signatures. The original includes a
FString construction/destruction around cache initialization at
`140738030/140692260`: the former zeroes16 bytes; the latter frees the first
pointer if nonnull. The fragment's ordinary FString locals supply that lifetime;
this is not an additional unclosed scoped-profiler dependency. The two explicit
config and logging bindings need compilation against the integration.

`policy.py` supplies bounded original-source checks, exact class and property
schema checks, and source accessors. The generator must invoke
`validate_source` and `validate_classes` before enabling `apply`, emit the
shared sources once, and use the existing `NativeGet144db86e0` declaration in
`startup_root_added_runtime/declarations.json`. The policy does not register
itself or alter generated files.

Evidence: bounded decode `144db86e0:453`, `141ed8a30:190`, `1406801e0:10`,
`142ee7880:59`, `142ee5dd0:5a`, `1405dcde0:43`, `142c475f0:70`,
`142c761d0:200`, and targeted UTF-16 reads of the referenced strings.
Candidate RIP-relative references were searched in bounded 1 MiB windows of
the code region, followed by targeted disassembly; no full-image load or hash
was used. The `0x40000000` internal object-array bit set in both paths is the
root-set bit.
