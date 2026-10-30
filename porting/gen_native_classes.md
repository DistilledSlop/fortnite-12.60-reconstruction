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

# 4. Self-test (33 checks, no files written).
python3 -I porting/gen_native_classes.py --self-test
```

`--order declaration` is the default (see below). `--order table` emits the
literal table order.

Inputs:

- `--name-map {block_va: "ClassName"}`. The only source of class names.
  Keys are the `block_va` strings written by the extractor (`"0x146b59ff0"`).
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
  `FFunctionParams` parameter table, when every candidate agrees.
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
- **Parent.** `base_fn` is taken as `dependency[1]` on the assumption that
  `dependency[0]` is the package singleton. That is consistent with AActor but
  has not been checked against any other named class. Only 307 of 2,740 parents
  resolve, and only through the type map. The `FFunctionParams.OuterFunc` value
  (0x1446ab350 for AActor's functions) might identify a block's own create
  function. Not confirmed: that value is not the `base_fn` of any block in the
  table, so it cannot be the create function of a class with children.
- **Struct layouts.** The struct's own `NetSerialize` and any custom bit format
  are not recovered. Structs are declared by name, but the generator does not
  write their definitions, and rep order inside a struct cannot be checked from
  the table alone.
- **Delegates.** Delegate signature types (`DECLARE_*_DELEGATE`) are not
  decoded. Unnamed or unresolved types are `TODO_*`.
- **Map and Set** element types decode when their inner records are present.
  FieldPath properties are `TODO`.
- **Function signatures.** Parameters use `T&` for out and reference params
  and `const T&` for const params. When the name-pointer join is ambiguous,
  the signature is `TODO` and the flags are missing.
- **Metadata.** Shipping builds have no metadata. Blueprint and editor
  specifiers, `BlueprintType`/`Blueprintable`, categories and `meta=` are not
  recoverable, so they are not emitted.
- **Bitfield bit order.** Only the width (1) is known. Bit positions do not
  affect replication, since `DOREPLIFETIME` uses the property name.
- **Not emitted:** function bodies, virtual qualifiers, includes, forward
  declarations and `GENERATED_BODY` beyond the minimum.

## Promotion to native/Source

Nothing here is promoted yet. The steps, once names exist:

1. Build the name map (`block_va` to class name) and the type map (`Z_Construct`
   function to C++ name). Sources to use: the other agent's Ghidra/Wine work
   and cooked package imports under `porting/generated/extracted/`.
2. Regenerate with `--name-map` and `--type-map`. Use `--all-groups` for engine
   parents such as `AActor`.
3. Review `TODO-LIST` in each header. Resolve every `TODO_*` token, the parent,
   delegates and missing includes by hand.
4. Check the generated declarations against the original `ClassChecksum` values
   for each class (see `original_client_net_protocol.md` section 2).
5. Copy each header to `native/Source/FortniteGame/Public/<Name>.h` and each
   `.cpp` to `native/Source/FortniteGame/Private/<Name>.cpp`. Replace any
   hand-written placeholder (for example `FortniteMapClasses.h`,
   `FortniteAthenaGameplay.h`) instead of duplicating the class.
6. Build `FortniteLinux` and `FortniteLinuxServer`, restage, and run
   `porting/server/smoke_join.sh`. Confirm the checksum lines match on both peers.
