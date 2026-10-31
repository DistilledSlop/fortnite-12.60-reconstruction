# gen_native_classes.py

Turns the reflection tables recovered from the shipping client by
`extract_reflection.py` into C++ scaffolds for UE 4.25: a UCLASS header with
UPROPERTY and UFUNCTION declarations, and a `.cpp` with
`GetLifetimeReplicatedProps`. Stdlib only. Output is a draft. Nothing is
compiled or promoted by this tool.

## Usage

```sh
# 1. Reflection JSON. The default run covers only Fort/Athena groups.
#    Engine classes such as AActor need --all-groups (33 MB, write to a scratch path).
python3 -I porting/extract_reflection.py
python3 -I porting/extract_reflection.py --all-groups --out <scratch>/all_groups.json

# 2. Generate. Only blocks present in the name map are written.
python3 -I porting/gen_native_classes.py \
  --reflection porting/generated/reflection/fortnite_game_reflection.json \
  --name-map <names.json> --type-map <types.json> \
  --out porting/generated/native_classes --api FORTNITEGAME_API

# 3. Compare one named class with a UE header (no files written).
python3 -I porting/gen_native_classes.py --reflection <json> --name-map <names.json> \
  --type-map <types.json> --compare-header <Actor.h> --compare-class AActor

# 4. Self-test (no files written).
python3 -I porting/gen_native_classes.py --self-test

# 5. Promote into a scratch tree (what promote_native_classes.sh does, but into <scratch>).
#    Behaviour snippets from porting/behavior are spliced by default; --no-behavior or FN_BEHAVIOR_SPLICE=0 turns
#    them off. Reports (skipped_properties.json, behavior_splice.json, ...) go to --report-dir.
python3 -I porting/gen_native_classes.py --promote --source-root <scratch>/src --report-dir <scratch>/reports \
  --reflection porting/generated/reflection/fortnite_game_reflection.json --name-map <names.json> \
  --evidence porting/generated/reflection/class_name_map.evidence.json \
  --parent-map porting/generated/reflection/class_parent_map.json \
  --class-ref-map <class_ref_map.json> --class-list porting/native_class_set.txt \
  --engine-classes /opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes [--param-order offset|table]
```

Behaviour snippets: `--behavior-dir` (default `porting/behavior`, its `MANIFEST.json`). The `inc_path` values are
relative to the repository root. See `porting/behavior/README.md` for what is spliced and what is rejected.

`--order declaration` is the default (see below). `--order table` emits the
literal table order.

Inputs:

- `--name-map {block_va: "ClassName"}`. The only source of class names.
  Keys are the `block_va` strings written by the extractor (`"0x146b59ff0"`).
- `--parent-map {block_va: {parent_name, parent_va, ...}}` from
  `porting/resolve_class_parents.py` (`porting/generated/reflection/class_parent_map.json`). When given it is the
  only source of the parent. See `binary_analysis.md` section 8.
- `--type-map {fn_va: "TypeName"}`. Maps the `Z_Construct_*` function
  pointers the extractor records (`type_refs`: class, meta_class,
  interface_class, struct, enum, and the parent `base_fn`) to C++ names.

Output under `porting/generated/native_classes/` (git-ignored with the rest of
`porting/generated/`): `<Name>.h`, `<Name>.cpp`, `index.json`. Every
unresolved reference is written as a `TODO_*` token that does not compile,
and each one is listed under `TODO-LIST` in the header. The output is
meant to fail until these are fixed.

## What is generated

- `UCLASS(config=..., defaultconfig, abstract, transient, minimalapi, deprecated)`
  from the class flags and config name. Parent from `base_fn` through the type map.
- UPROPERTY in declaration order, typed from `EPropertyGenFlags` and the
  pointers that follow the offset: `TArray`, `TMap`, `TSet`, `TSubclassOf`,
  `TSoftClassPtr`, `TWeakObjectPtr`, `TLazyObjectPtr`, `TSoftObjectPtr`,
  `TScriptInterface`, `TEnumAsByte`, enum classes, and structs and classes
  by name. Bool members are `uint8 X : 1` when the bitfield flag is set,
  otherwise `bool`. Static arrays use `[N]`.
- Specifiers: `Replicated` for `CPF_Net`, `ReplicatedUsing=<fn>` when a
  RepNotify name exists, plus `Transient`, `Config`, `SaveGame`, `Interp`.
  Every declaration carries its raw property flags as a comment.
- UFUNCTION with `Server`, `Client`, `NetMulticast`, `Reliable`, `Exec`.
  `static` and `const` follow the function flags. Signatures come from the
  `FFunctionParams` parameter table, when every candidate agrees, in declaration order (see "Parameter order").
- `GetLifetimeReplicatedProps` with `DOREPLIFETIME` in table order, calling
  `Super::` when the parent resolves.

## Property order

Observed in the AActor table (not assumed): the table lists members in the
reverse of their declaration order in `Actor.h`. 73 of 75 members match after
reversal. `bHidden` and one of `bCanBeDamaged`/`bBlockInput` are out of place.
The cause is unknown; it may be a difference in the 12.60 engine's header.

The generator therefore emits declarations in declaration order, which is the
table reversed, so that UHT reproduces the original table. `--order table`
gives the literal table order and matches only 2 of 75 declarations. `DOREPLIFETIME`
lines are always in table order, as specified.

## Parameter order

The parameter table lists the parameters of a function or delegate in reverse declaration order. UHT emits its
`PropPointers` from the last parameter back, and each parameter's recorded `offset` is `STRUCT_OFFSET` of the compiled
`Parms` struct, which lays members out in declaration order. So the offsets ascend in declaration order and the table
reads descending. Checked on the reflection: the recorded offsets of the parameters are strictly descending in table order for every
function and delegate (4,492 functions and 331 delegates with two or more offsets; the only exceptions are the enum
underlying-type and map key/value records, which are not parameters), and the binary agrees: the thunk of
`AFortPlayerPawn::ServerHandlePickup` (`porting/generated/decomp/AFortPlayerPawn/ServerHandlePickup.c`) reads the wire
parameters at offsets 0, 8, 12 and then the bool, and calls `_Validate` with them in that order.

`--param-order offset` (the default, also `FN_PARAM_ORDER=offset`) spells parameters in declaration order, so the
generated signatures, the `DECLARE_*_DELEGATE*` macros and the `Parms` layout follow the recorded offsets. A list whose
recorded offsets are not in reverse table order is kept in table order and reported as a warning (zero in the current
tables). `--param-order table` (`FN_PARAM_ORDER=table`) reproduces the earlier spelling. Behaviour snippets whose manifest
signature is spelled in table order are then rejected by the splice ("manifest signature ... differs"), so the manifest
must be spelled to match the selected order.

Delegates also changed in this order. UHT marks every reference parameter with `CPF_ReferenceParm` and `CPF_OutParm`,
so a const reference (`const T&`) carries `Parm|Out|Const|Ref`. Such a parameter is now emitted as `const T&` (or
`T* const&` for a pointer). Before, it was rejected as "return, out or reference parameter", which dropped 100
`TODO_DelegateNotEmittable` members on the committed class set of `3bb86bd` (91 are emitted now; the other 9 need parameter
types that are still unavailable; on the merged set of `main` 115 of 167 are emitted now, 52 remain).
A return parameter and a non-const out or reference parameter are still rejected.

An enum's underlying-type record (named `UnderlyingType`, directly after the Enum entry) is paired with the enum for
both `Byte` and `Int` underlying types. The extractor marks only the `Byte` one. Before, an `Int` one became a bogus
`int32 UnderlyingType` member or parameter (for example in `ServerRemoveMapMarker`), and its struct could not be emitted.
This fix applies in every parameter order.

## Validation: AActor

Block `0x146b59ff0` (config `Engine`, `CLASS_Config`), named `AActor` with a
synthetic name map. Parent `UObject` (`base_fn` 0x1446fdba0). Type-map entries
for the class and struct pointers were seeded by hand from `Actor.h`, so the
type results check the decoding rules and pointer identity, not the names.
The seeding also checks identity: `Owner` and `Children` share one class
pointer; `Role` and `RemoteRole` share one enum pointer.

| Check | Result |
|---|---|
| Property names | 75 of 75 non-conditional `Actor.h` UPROPERTYs present, none extra. 17 declared under `#if WITH_EDITOR` or `WITH_EDITORONLY_DATA`, absent from the shipping table as expected. |
| Replicated set | 10 of 10: Role, RemoteRole, Owner, AttachmentReplication, ReplicatedMovement, bReplicateMovement, bTearOff, bCanBeDamaged, bHidden, Instigator. 0 replication mismatches, including the 5 RepNotify names. |
| Types | 59 of 75 match exactly. The other 16 are sparse delegates (`FActorEndPlaySignature` etc.). The reflection gives only the delegate name, not its signature type, so they stay `TODO_Delegate_*`. |
| Order | 73 of 75 match the reversed table (see above). |
| UFUNCTION | 127 generated vs 128 in the header. The header-only `SetAutoDestroyWhenFinished` is absent from the table. 0 net-role mismatches, but AActor has no net UFUNCTIONs, so this check is trivially met. |
| Parent | `UObject` from the seeded map. Not verified against any independent source. |

Robustness: all 2,740 Fort/Athena blocks generate with synthetic names in
about 0.6 s, with no exceptions.

## Known limits

- **Names.** No block has a name yet. Every name comes from `--name-map`.
  Unnamed blocks produce no output.
- **Parent.** `base_fn` is `dependency_fns[1]`, which is the module package, not
  the parent. The parent is `dependency_fns[0]` (UHT emits super first, package
  last). Use `--parent-map`. Without it the generator falls back to `base_fn`
  and will usually emit a TODO parent. Five CoreUObject intrinsic parents
  (UClass, UEnum, UScriptStruct, UPackageMap, UInterface) have no block and stay
  unresolved in the map; their names come from child headers.
- **Struct layouts.** Recovered by `porting/extract_types.py` (`binary_analysis.md` section 9). Draft mode
  still writes no struct definitions and keeps unresolved structs as `TODO_*`. The native net serializers of
  structs are described in "Net serializers" below.
- **Delegates.** Signatures are recovered (`delegate_blocks.json`). Draft mode keeps delegate types as
  `TODO_*`; promote mode emits the `DECLARE_*DELEGATE*` macro for each delegate used, parameters in declaration order
  (see "Parameter order"). A delegate that still cannot be emitted is listed in `skipped_properties.json` as
  `delegate_member_unresolved`, with the unavailable type in its `tokens`.
- **Map and Set** element types decode when their inner records are present.
  FieldPath properties are `TODO`.
- **Function signatures.** Parameters use `T&` for out and reference params
  and `const T&` for const params, in declaration order. When the name-pointer join is ambiguous,
  the signature is `TODO` and the flags are missing.
- **Metadata.** Shipping builds have no metadata. Blueprint and editor
  specifiers, `BlueprintType`/`Blueprintable`, categories and `meta=` are not
  recoverable, so they are not emitted.
- **Bitfield bit order.** Only the width (1) is known. Bit positions do not
  affect replication, since `DOREPLIFETIME` uses the property name.
- **Not emitted:** function bodies (except the spliced behaviour snippets), virtual qualifiers, includes beyond
  those the emitted types need, forward declarations beyond the emitted types, and `GENERATED_BODY` beyond the minimum.

## Replication conditions and net serializers (promote)

- `GetLifetimeReplicatedProps`: `--promote` reads `porting/replication_conditions.json` (`--replication-conditions`; a
  missing file gives plain `DOREPLIFETIME`). A recovered condition is `DOREPLIFETIME_CONDITION(C, P, COND_X)`, or
  `DOREPLIFETIME_CONDITION_NOTIFY(C, P, COND_X, REPNOTIFY_Always)` when the registration has the always-notify flag.
  A property whose registration was not recovered is `DOREPLIFETIME(C, P);` with a `TODO` comment (COND_None assumed).
  Medium and low confidence properties carry a comment with their confidence. `bIsPushBased` is recorded in the data
  but not emitted (its native bodies are not recovered). Method and validation: `porting/decompilation_notes.md`,
  "Sixth pass".
- Net serializers of structs (`render_struct_header`, `net_serializer`):
  - An `FFastArraySerializer` whose TArray member holds `FFastArraySerializerItem` structs gets the native
    `NetDeltaSerialize` (`FFastArraySerializer::FastArrayDeltaSerialize<Item, Self>` over that member) and
    `TStructOpsTypeTraits<F...>` with `WithNetDeltaSerializer = true`. Include: `Engine/NetSerialization.h`.
  - Any other struct with a custom `NetSerialize` (or shared serialization) is declared as a fallback: no trait, and
    a `TODO` comment. Replicated uses serialize with the default property serialization.
  - Replicated properties and RPC parameters of such structs are emitted (they were skipped before). That is what makes
    the replicated class layouts match the original (`class_checksum.py`).

## Promotion to native/Source (first pass, done)

`porting/promote_native_classes.sh` runs `gen_native_classes.py --promote` on the closure in
`porting/native_class_set.txt`. It writes `native/Source/<Module>/Generated/<Name>.{h,cpp}` (package
`/Script/FortniteGame` -> `FortniteGame`, `/Script/PlayspaceFramework` -> `PlayspaceFramework`) and the reports
`porting/generated/reflection/skipped_properties.json` and `native_classes_index.json`. Status and blockers:
`porting/native_classes_status.md`.

Promote mode differs from the draft in these ways:

- A property whose type does not resolve is skipped and recorded (struct, enum, delegate, interface, or a class
  outside the native set). Nothing is emitted with a `TODO_*` token; the script refuses to write one.
- Class references are resolved through `resolve_class_refs.py`: the reference points at a `jmp` thunk to the class's
  NoRegister function, and the thunk is followed in the runtime dump.
- Functions: only net UFUNCTIONs (RPCs), RepNotify targets and the non-RPC functions named by a ported behaviour snippet
  are emitted. An ambiguous name join is resolved to the candidate whose `outer_fn` is the class's own construct
  function (the extractor keeps the candidates).
- Each emitted RPC gets an `_Implementation` stub (UHT requires it), and each RepNotify gets an empty body. A ported
  snippet replaces the stub body, and a validate snippet adds `WithValidation` and `<Rpc>_Validate` (see the splice
  section of `porting/behavior/README.md`).
- Net service functions (flags `FUNC_NetRequest` / `FUNC_NetResponse`, no Server/Client/NetMulticast role) are
  emitted as `UFUNCTION(ServiceRequest(...))` / `UFUNCTION(ServiceResponse(...))` (`net_service_kind`). They have no
  `_Implementation`. A request gets no `.cpp` definition (UHT generates it, see below). A response is the programmer's body,
  and its stub is empty (with `return {};` when it returns a value).
  The endpoint is a placeholder (`NET_SERVICE_ENDPOINT`); the response ids are `NET_RESPONSE_IDS`, read from the dump
  because the reflection tables do not carry `RPCId` / `RPCResponseId`. A response id needs `JSBridge` as the endpoint
  (UHT, HeaderParser.cpp). Request ids are 0 in the dump.
- A service request's out parameter (`FClientUrlContext& Context`: CPF_OutParm and CPF_RepSkip, no CPF_ReferenceParm in the
  reflection) is declared `UPARAM(NotReplicated) FClientUrlContext& Context` in the header (`header_arg`). UHT rejects any
  non-const reference on a net function unless the parameter is NotReplicated (HeaderParser.cpp, "unless marked
  NotReplicated"), and the flags then match the original.
- A service request has no `.cpp` definition: UHT generates its Event thunk (a `ProcessEvent` forwarder, found in
  `Intermediate/.../Inc/<Module>/<Class>.gen.cpp`) for every request. The first build failed at link with duplicate
  symbols, 138 of 138 requests (`render_promoted_cpp` skips them; a spliced body raises).
- A const RPC is emitted with `const` on the declaration and on its `_Implementation` (UHT sets `FUNC_Const` for it).
- UCLASS has no `minimalapi` (UHT rejects it with the module API macro).

Promote mode also emits the struct, enum and delegate types used by the emitted members and RPCs (transitively), one
header per type under the module's `Generated` directory (`F<Name>.h` for a struct, the enum or namespace name for an
enum, `F<DelegateType>.h` for a delegate). Engine types are not emitted; they are included from the engine tree. The
type tables come from `--types-dir` (default: the reflection directory). `type_closure.json` lists the emitted types,
the engine types included, and the promoted types that cannot be emitted with the reason.

The draft mode (no `--promote`) is unchanged and still writes `TODO_*` tokens into `porting/generated/native_classes`.

## Cross-module types and engine enums (class checksum pass)

- A FortniteGame item may use a struct or enum of a runtime module in `CROSS_MODULE_TARGETS` (every runtime module
  except FortniteUI, which depends on FortniteGame). `cross_module_allowed` decides it: the header is included, and the
  module is added to the item's module dependencies, which the game Build.cs must list (promote checks it). The
  reverse edge is never allowed, so the module graph stays acyclic (promote still refuses a cycle).
- Namespaced engine enums (`namespace ECollisionEnabled { enum Type }`, named `ECollisionEnabled::Type`) are indexed by
  `NAMESPACE_ENUM_RE`. `ENGINE_SCAN_DIRS` also scans `Plugins/Online/OnlineSubsystemUtils` (the party and spectator
  beacon state types). The other Online plugins stay out: they would add `WebBrowser` and `LoginFlow` includes.
- The game Build.cs lists `WebBrowser` (the JS-bridge types of UFortGameInstance); runtime module Build.cs files inherit
  the game module's engine list, so they list it too.
