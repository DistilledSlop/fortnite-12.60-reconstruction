# Director local-player root source candidate

This directory supplies generator-input C++ for director virtual `280`, its
actual class selection body `145176990`, manager add `145146bc0`, real root
creation `14514ab50`, attachment `145146470`, and the player callback body
`14514cba0`. No generated source or Engine code was changed. Source names with
addresses are aliases, not recovered C++ symbols. No normal startup success,
compilation, or runtime verification is claimed.

The candidate preserves selection, synchronous manager `LayoutClass` loading,
this player's controller in the manager's world, `CreateWidget` with that
controller and NAME_None, reflected player/root storage, the native attached
flag, `SetPlayerContext(FLocalPlayerContext(LocalPlayer))`,
`AddToPlayerScreen(0)`, and dispatch of the manager's real root-added hook.
There is no result-null early return after widget creation in the original.
Adding one would change this method's behavior. The callback is weakly bound
to the manager while capturing both manager and local player pointers.

Important correction to the earlier evidence prose: reflection block
`14700d9a0` names Entryway path/class at `48/90`, Athena at `60/98`, and
Campaign at `78/a0`. The earlier Campaign/Entryway ordering is reversed.
Selection returns null without a FortLocalPlayer. Otherwise, when both player
predicate `14238f060` and global `141f3ad70` are false, it selects Entryway.
The global is a cached `FParse::Param(FCommandLine::Get(), TEXT("nomcp"))`:
literal `145bc3a00`, accessor `141f3ad70`. In the remaining branch GI byte
`828` selects 2=Entryway, 1=Athena, 0=Campaign, other=null. The setter is called
only if the validated previous and selected classes differ and GI byte `7bb`
is zero. It still receives the original selected pointer and validates again.
No source enum name or byte-7bb field name has been invented.

The present generated `FFortRootViewportLayoutInfo` has two reflected pointers
and omits native `bool +10`: the original array uses stride `18`. The generator
must retain its reflected GC-visible pointers and append this native bool.
The base manager constructor `145143bd0` defaults legacy input false, empties
this array, clears byte `68` and the FGameplayTag at `6c`, and constructs its
native delegates. The constructor is not supplied piecemeal because those
additional native fields and delegate contracts are not yet represented.

The immediate blocker is real manager lifecycle, not the root factory:

- `1451746d0` creates managers with director Outer, after old-manager shutdown,
  captures/re-adds attached players, restores interface state, and broadcasts
  director multicast `30`. Replacing it with `NewObject` alone is incorrect.
- Startup root-added virtual `298` resolves to `145166f50` in installed table
  `147011440`. It propagates current state via root `490`, inserts the player
  and root into manager `PlayerWidgets` (`c0`), derives root state/input gates,
  calls `14516e780` and `145152480`, and broadcasts FortGI native multicast
  `5d8`. The root-removed slot `2a8` is `145167210`, which removes its map entry.
  A no-op root-added hook would discard actual startup behavior.
- State refresh `1451755b0` resolves the first player's controller context,
  calls manager virtual `280` (Startup `14514b4b0`), honors the byte-7bb hold,
  compares/stores tag `6c`, and invokes state-change virtual `290` when changed.
- The player native multicast `348` producer and exact signature still need
  source typing before the `AddWeakLambda` declaration can be promoted. The
  captured callback's executed body and weak-object behavior are recovered.
- Manager `GetWorld` is now recovered in manager-world.definitions.inc by the
  primary reviewer: virtual150 ->14514f610 gets the director Outer, casts that
  director's Outer to UGameInstance through14460bc60, then directly calls
  UGameInstance::GetWorld at144217e60. Getter144706480 names GameInstance. No
  extra null fallback is present. UObject's existing GetWorld forwards to its
  Outer; it is not a constant-null function. Integrate the exact override with
  the manager lifecycle rather than assuming an arbitrary world provider.

`contract.json` freezes 19 bounded source/type windows, four installed virtual targets, and
full reflection records for five classes. `policy.validate_original` and
`validate_classes` verify those inputs. `fragments()` exposes generator-ready
text; `apply()` deliberately rejects promotion until the listed real
dependencies exist. No fake provider, stub manager, raw-offset memory access,
forced gate, or diagnostic widget launch is supplied.

Validation command (PASS): import policy, validate the immutable mapped image,
validate its class records against the original reflection/name map, and load
the fragments. Engine source verification: UPlayer::GetPlayerController in
`Engine/Private/Player.cpp:73` exactly matches helper `14447f220` (null-world
member fallback; otherwise iterate world controllers and match local player).
This is bounded provenance/source review only. The next bounded implementation
should close manager state/update and Startup root-added lifecycle, beginning
with `145166f50`, `145152480`, `14516e780`, and real storage/providers.
