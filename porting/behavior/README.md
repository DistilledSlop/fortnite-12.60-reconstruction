# Behaviour snippets (porting/behavior)

Hand-ported native function bodies recovered from the shipping 12.60 client's decompile
(`porting/generated/decomp*/`, git-ignored). Each `.inc` is our own C++ re-expression of what the
original does, not a copy of the decompile.

`MANIFEST.json` is the single index: one entry per function (4,117 ported, 148 skipped with a reason;
`COUNTS.txt` has the summary). Every ported entry points at its body through `inc_path`, and the
layout is fixed:

- `<ClassName>/<FunctionName>.inc` holds the body of that function.
- `_shared/*.inc` holds bodies that are byte-identical across many entries (empty `_Implementation`
  stubs, `_Validate` returning true, trivial returns); many entries may share one path.
- Anything not in `MANIFEST.json` is not part of the port. Do not add per-batch or per-session
  directories here: add or edit the manifest entry and its `<Class>/<Function>.inc` file, then run
  `python3 -I porting/behavior/check_behavior.py`.
- Evidence for a body belongs in the `.inc` header comment (original address, decompile source,
  confidence) and in the entry's `notes`. Long-form findings live in `porting/decompilation_notes.md`
  and the `*_evidence.md` files next to this README. Per-pass history is in git.

Frontend (`UFortRuntimeOptions`) background: `GetRuntimeOptions` returns the class default object
(`GetMutableDefault<UFortRuntimeOptions>()`); `GetGameVersion` derives its result from the runtime
build/branch strings and `FEngineVersion` rather than a constant; `GetContext` still depends on
unresolved context lookups. See `porting/frontend/` for the probes and their evidence.


## Files

- `MANIFEST.json`: array of entries `{class, function, kind, signature, signature_status, inc_path,
  confidence, address, status, proxy_score, decompile_source, notes}`. One entry per function, even when
  several functions share one body.
  - `status`: `ported` (has an `inc_path`) or `skipped` (`inc_path` is null, `notes` holds the reason).
  - `kind`: `getter`, `onrep` (RepNotify body), `impl` (RPC `_Implementation`), `native_event_impl` (BlueprintNativeEvent `_Implementation`), `validate` (RPC `_Validate`),
    `exec` (other BlueprintCallable body), `callee` (an unidentified helper queued in CALLEE_PORT_QUEUE.md),
    `helper` (a non-reflected member function), `static_helper` (a static free function in the class .cpp),
    `member_field` (a non-reflected data member) and `struct_net` (the NetSerialize of a struct); see "Helpers",
    "Member fields" and "Native NetSerialize (struct_net)" below.
  - `signature_status`: `declared` (copied verbatim from the generated header), `proposed` (not in the header;
    the generator must emit it), or `not_declared` (skipped entries).
- `<ClassName>/<FunctionName>.inc`: one snippet per ported function with its own header comment.
- `_shared/validate_return_true.inc` and `_shared/impl_empty.inc` (plus the other `_shared/*.inc` files): one body each for the folded stubs.
  - `0x1406801d0` (`mov al,1; ret`, returns true) is the body of 71 ported `_Validate` functions.
  - `0x1406801c0` (`ret 0`, empty) is the body of 49 ported `_Implementation` functions. Four more with the same
    body are skipped because the generated tree does not declare them.
  - Each manifest entry names the shared address in its `notes`.
- `bool_bits.py`: maps every bool property to its byte offset and bit mask from its SetBit function. It imports
  `porting/class_checksum.py` (read only) and writes `porting/generated/reflection/bool_bits.json` (git-ignored).
  All 6,576 bool setters decode. The map cross-checks the decompile: `IsDBNO` reads byte +0x53a bit 0x2, and the
  setter for `bIsDBNO` is `or byte [rcx+0x53a], 2`.
- `BLOCKING_CALLEES.md`: ranked list of the unidentified `FUN_*` callees that block the most functions
  (identified helpers from `helpers.json` are removed), guesses from each callee's own bytes, and the RPC thunk
  statistics.
- `CALLEE_PORT_QUEUE.md` (from the callee identification pass): its 17 rows are listed as skipped `callee` entries.
- `check_behavior.py`: stdlib checker (see below).

## The .inc contract

- A `.inc` file is the body only: the statements that go between the braces of the native member function.
  No signature, no braces around the whole body, no `#include` lines. An empty body is a file with only comments.
- It runs inside the generated member function, so `this` members are available as bare names and `this->X`
  is valid. Parameters use the names from the signature.
- Non-void functions use `return`. Void functions just end with their statements.
- Members are looked up in the class and in its generated ancestors. Bitfield and native bools are read and
  assigned by their header name.
- Engine names allowed without a generated header (checker `ENGINE_NAMES`): `UClass`, `UObject`, `AActor`,
  `FWeakObjectPtr`, `TWeakObjectPtr`, `FName`, `FString`, `FText`, `FVector`, `TArray`, `static_cast`, `Cast`, and (ninth
  pass) `UWorld`, `AGameStateBase`, `GetLocalRole` (the public accessor of the private `AActor::Role`) and `ROLE_Authority`.
  Tenth pass: `GetOwner` (the public accessor of `AActor::Owner`, which is not declared in any generated header).
  Eleventh pass: `FMath` (`FMath::Abs` in the struct_net body of `FCreateBuildingActorData`). The C library call
  `::nearbyintf` is `::`-qualified, so the checker does not see it.
  `GetWorld` is the inherited `UObject` virtual, also declared outside the generated class chain.
  Aircraft lookup also uses `UE_LOG`, the shared `LogFort` category, and its `Verbose`/`Warning` levels.
  The name is accepted by the checker only; the header is still needed. A getter or exec can declare validated engine
  `includes` in the manifest; the generator emits those in the corresponding `.cpp`.
- Each file starts with a `//` header comment: original address, decompile source, proxy score, what was
  inferred and `CONFIDENCE: HIGH|MEDIUM|LOW`. The checker requires the tag to match the manifest.

## Splice (implemented in gen_native_classes.py)

`gen_native_classes.py --promote` splices the ported entries (`status == "ported"`, 199 today). It is on by default.
Turn it off with `--no-behavior` or `FN_BEHAVIOR_SPLICE=0`; the output then equals the stub output. Per entry:

- `getter`, `exec` (`signature_status: proposed`): the non-RPC reflection function is emitted as
  `UFUNCTION(BlueprintCallable[, BlueprintPure])` (BlueprintPure when the reflection flag is set), with the manifest
  signature (`const` included) and the snippet as body. For a reflected `FUNC_Static` function the manifest must have
  `"static": true`; the generator verifies staticness and emits `static` on the declaration only. Static-and-const
  reflection is rejected, as are static bodies using `this` or an unqualified instance member known to this class.
  These textual checks do not prove that a body compiles or catches inherited instance members.
  A getter or exec may list explicit engine `includes`; the generator validates these against its usable engine
  header index and includes them in the generated `.cpp`, not the header. Apart from the helpers (see "Helpers" below), only these two kinds add functions that are not RPCs.
Parameter order (generator change, see `porting/gen_native_classes.md`, "Parameter order"): a signature lists its
parameters in declaration order, the order of the recorded offsets. The table lists them in reverse, so a manifest
signature written in table order is rejected. In this pass the 27 `impl` and `validate` signatures with two or more
parameters were corrected to declaration order, and `AFortPlayerPawn::GetPreviousPosition` and `GetPreviousVelocity`
lost `const` (the reflection flags have no `FUNC_Const`, and the snippet bodies do not depend on it). Each correction
is the reflection-derived signature the generator prints in `behavior_splice.json` for that entry. With them all 199
ported entries splice and verify (`--gen-root ... --splice-report`: 199 spliced, 0 rejected, FAIL 0).

- `validate` (`<Rpc>_Validate`): the Server RPC gets `WithValidation`, `bool <Rpc>_Validate(<same params>)` is declared
  and the snippet is its body. The RPC must be a Server RPC; otherwise the entry is rejected.
- `impl` (`<Rpc>_Implementation`) and `onrep` (RepNotify): the snippet replaces the stub body. Declarations already exist.

The generator splices an entry only when its signature equals the reflection-derived one (whitespace ignored), the
RPC or function is emitted, the body has balanced braces and no preprocessor line, and every class or type the body
names is in this module or an engine module with a known include. Other entries are rejected: nothing is emitted for
them and they are listed in `behavior_splice.json` (in the report directory) with the reason. Snippet includes go into
the `.cpp` (types used by the body); types used by a signature go into the `.h` as before. After the text is rendered,
every spliced function is checked for one declaration, the UFUNCTION line in front of it, one definition with the same
signature, and a body equal to the snippet; a mismatch stops the run before anything is written.

Snippet bodies keep their comment header as written. The "Declared in generated header" lines were refreshed in the fourth pass (no: the generator emits the declaration once the splice is on; yes: the header has it).

## Helpers (kinds `helper` and `static_helper`)

A helper is a function that no reflected UFUNCTION owns, so the generator writes its declaration itself. `helper` is a
plain C++ member of the class. `static_helper` is a static free function in the class's .cpp. Both are `status: "ported"`
with an `inc_path`, and `signature_status: "proposed"` until the generator emits them. A helper that was queued in
`CALLEE_PORT_QUEUE.md` is ported by adding its entry here; the queue row is not edited by the generator.

Manifest fields, in addition to the usual ones (`class`, `function`, `signature`, `inc_path`, `confidence`, `address`,
`status`, `decompile_source`, `notes`):

- `signature`: the C++ declaration without `;`, and without `static`, `virtual` or `inline` (the kind sets the linkage).
  The return type is spelled as it is written (`UFortPlaylistAthena* GetCurrentPlaylistAthena() const`), then the name,
  then the parameter list. Every parameter has a type and a name. `const` is allowed only on `helper`.
- `virtual` (`helper` only, optional boolean): emit a native C++ virtual member when original vtable evidence establishes dispatch. The signature still omits `virtual`; generation and the checker reject loss of the keyword. This does not make the helper a Blueprint event.
- `override` (`helper` only, optional boolean): emit and verify `override` on the declaration; omit it from the signature and definition. The C++ compiler checks the inherited virtual signature. Static helpers cannot use it.
- `access` (`helper` only, required): `public` or `protected`. A `static_helper` takes no access. A helper that another
  class calls must be `public` (a `protected` member is not visible to that class).
- `includes` (optional): a list of header spellings (for example `Engine/World.h`) for types that the signature or the
  body needs and the generator does not resolve itself. Each spelling must be in the engine header index of the usable
  modules, otherwise the entry is rejected. The headers go into the .cpp, and into the .h for a member helper.

Types in a helper signature (and body names, as for the other kinds):

- Fundamentals and CoreMinimal names need no include: `void`, `bool`, `int8`..`uint64`, `float`, `double`, `TArray`,
  `TMap`, `TSet`, `TSubclassOf`, `TWeakObjectPtr`, `TSoftObjectPtr`, `TSoftClassPtr`, `TEnumAsByte`, `FString`, `FName`,
  `FText`, `FGuid`.
- A struct or enum of a promoted module is included by the generator.
- A class of this module is forward declared, and it must appear as a pointer or a reference (`UFoo*`, `const UFoo&`).
  The generator adds the class header to the .cpp only when the body names the class. A body that dereferences such a
  parameter must therefore name its class (the ninth pass writes a typed local, `const UFortPlaylistAthena* ScoringPlaylist
  = Playlist;`); a comment does not count.
- Anything else (an engine class such as `UWorld`) must be listed in `includes`, or the entry is rejected.
- A class of another module is rejected, as for bodies.

The body (`.inc`) follows the contract above. A member helper uses the object's members as bare names. A `static_helper`
has no object: its body uses its parameters, its locals, its types and other static helpers, and it must not use `this`,
bare class members or bare class member functions. Dependencies: a body may name other helpers of the same class. A
member helper may use any emitted helper. A `static_helper` may use only static helpers, and it is rejected if it names
a member helper. A body (of any kind) that names a helper that is not emitted is rejected. The static helpers are
emitted dependencies first. A cycle among static helpers rejects every static helper in or behind the cycle.

Where the generator writes them (with `--promote`, when the splice is on):

- `helper`: the declaration goes into `<Class>.h` after the reflected members and `GetLifetimeReplicatedProps`, in a
  block headed by a comment, outside any UFUNCTION declaration. Its access label (`public:` or `protected:`) is written
  when it changes; the block starts public, and public helpers come first. The definition goes into `<Class>.cpp` after
  the reflected functions.
- `static_helper`: the definition goes into `<Class>.cpp` in `namespace { ... }  // namespace`, before every member
  definition, written `static <ret> <name>(<params>)`. There is no header declaration.

Checks: `verify_spliced` in the generator checks each helper's declaration (once, not under a UFUNCTION, under its access
label), its definition (once, with the snippet as body) and the namespace position. `check_behavior.py --gen-root` checks
the same on a generated tree, and the field rules of both kinds (`check_kind`). The worked example is
`AFortGameStateAthena::GetCurrentPlaylistAthena` (kind `helper`, `public`, queue row 5, renamed from `GetActivePlaylist` in
the ninth pass): it returns `CurrentPlaylistInfo.OverridePlaylist` when that is non-null, else `CurrentPlaylistInfo.BasePlaylist`.
The static helper example is `GetFortGameStateAthena` (kind `static_helper`, queue row 4). It takes its world-context
object as a parameter and lists its engine headers in `includes` (`UObject/Object.h`, `Engine/World.h`,
`GameFramework/GameStateBase.h`), because its body names `UWorld` and `AGameStateBase`. The generator writes its
definition in the .cpp of the class that owns the entry, so the other classes that call it need their own entry.

## Member fields (kind `member_field`)

A member field is a plain data member of a generated class that no UPROPERTY declares: a native field such as a cache, or
a native `TMulticastDelegate` list. It has no body, so it has no `inc_path` (an entry with one is rejected). Its fields:

- `signature`: `type name`, without `;` and without the initialiser. The type is spelled as in C++, template arguments
  included (`TMulticastDelegate<void, AFortPlayerStateAthena*, int32> KillScoreNative`: UE 4.25 spells a multicast
  delegate with the return type and the parameter types as template arguments, not as a function type; the first
  spelling was rejected by the compiler in the ninth pass). `static`, `mutable`, `volatile`,
  `virtual`, `inline`, `extern`, `typedef`, `using` and `UPROPERTY` are rejected. The name is the entry's `function`.
- `access` (required): `public`, `protected` or `private`.
- `default` (optional): one expression, without `;`, `{`, `}` or a newline; a number is written as its text. Rendered as
  `type name = default;`.
- `offset` (optional, informational for the layout): an integer or a string such as `"0xe88"`. A field with an offset is
  placed in declaration order among the reflected members: before the first reflected member whose offset is larger. A
  field without an offset goes after the reflected members, in manifest order. Placement is the only use of the offset;
  the bodies name the field, not its offset.
- `includes` (optional): header spellings for types the generator does not resolve, as for helpers (for example
  `Delegates/Delegate.h` for `TMulticastDelegate`).

Type rules are those of helper signatures: a class of this module must be a pointer or reference, a struct or enum of a
promoted module is included, and any other name needs `includes`. A field name must not repeat a reflected member, a
helper or a function of the class.

Output: the declaration goes into `<Class>.h` (no definition in the .cpp), under its access label. A non-public field is
followed by `public:`, because the reflected members that follow are public. The declaration is outside any UPROPERTY and
UFUNCTION, and the field is not replicated. The generator's `verify_spliced` checks the declaration (once, outside a
UPROPERTY or UFUNCTION, under its access label) and the offset order. `check_behavior.py` checks the same in `--gen-root`
mode, and in plain mode reports the field declared or pending. Helper bodies may name a field of their class as a bare
name, as for reflected members; the plain token check accepts the names of the manifest's member fields.

## Native NetSerialize (kind struct_net)

**UE 4.25 bool rule:** in a non-editor build `FArchive::SerializeBool` does not exist (it is `#if WITH_EDITOR` and protected, Archive.h ~line 1228). Write `Ar << MyBool;` instead: `operator<<(FArchive&, bool&)` serializes a 32-bit UBOOL, the same bytes the editor-only `SerializeBool` writes and the original binary's body produced. (Found by the struct_net build, which failed with 158 `no member named 'SerializeBool'` errors until the two affected snippets used `Ar << x`.)

A struct with a custom `NetSerialize` has no UFUNCTION and no `.cpp`, so its body cannot use the other kinds. The kind
`struct_net` splices the recovered body into the struct header. Its fields:

- `class`: the struct's C++ name (`FSharedRepMovement`); `function`: `NetSerialize`; `status`: `ported` with an `inc_path`
  (`porting/behavior/<Struct>/NetSerialize.inc`), `address` (the body) and the thunk and vtable in `notes`.
- `signature`: exactly `bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)`. No `access`.
  `signature_status`: `proposed` (the generator emits the declaration).
- The body is the usual `.inc` contract (statements only, `CONFIDENCE` tag matching the manifest, no preprocessor line).
  `Ar`, `Map` and `bOutSuccess` are the parameters. Members of the struct are bare names; the struct's parent struct is
  visible (checker chain).

Where it goes (`gen_native_classes.py`, `TypeGate.native_net_serializer`, `render_struct_header`, `verify_struct_net`):

- The traits come from the binary, not from the manifest: the struct must have `HasNetSerializer`, and
  `HasNetSharedSerialization` is optional. A struct with `HasNetDeltaSerializer` is not a candidate. The header then has
  `TStructOpsTypeTraits<S>` with `WithNetSerializer = true` and, when the binary has it, `WithNetSharedSerialization = true`.
- The declaration is inline in the struct, at one tab; the body is at two tabs. `class UPackageMap;` is forward declared
  and `Serialization/Archive.h` is included (FArchive must be complete for the inline body).
- Types the body names (for example `EFortSharedAnimationState`) are emitted with the struct, and their headers are
  included; a body that names a type of another module, or a type that cannot be emitted, is rejected.
- A struct that is not emitted, a second entry for one struct, a different signature or a body that does not
  pass the checks is rejected (listed in `behavior_splice.json`). The struct then keeps the default serialization and
  its TODO comment (the fallback).
- `verify_struct_net` checks the rendered header as text (declaration once inside the struct, body equal to the snippet,
  one trait block, the forward declaration) before anything is written. `check_behavior.py --gen-root` checks the same
  on a generated tree.
- Not a `struct_net` entry: `FAthenaPawnReplayData` (its wire depends on runtime state and key handling; no entry).

The five bodies were checked against the generated headers (member names and types), against the UE 4.25 archive
API (`Core/Public/Serialization/Archive.h`: `Serialize`, `SerializeBits`, `SerializeInt`, `SerializeIntPacked`, virtual
`SerializeBool(bool&)`) and against the engine's `FRepMovement::NetSerialize` (`Engine/Classes/Engine/EngineTypes.h`).
Rounding in `FCreateBuildingActorData` is `::nearbyintf` (round to nearest, ties to even), not `FMath::RoundHalfToEven`,
which snaps values near a half. None of the bodies is compiled by the checker: the build owner compiles them (Linux
toolchain: `-Wall -Werror`, no `-Wextra`).

## Checker

`check_behavior.py` (stdlib; `python3 porting/behavior/check_behavior.py [--verbose]`):

- signature: `declared` entries must appear in the generated header; `proposed` entries count as declared once the
  generator emits them (`declared_in_header`) and are reported as `PENDING_DECL` until then;
- `--gen-root <dir>/FortniteGame/Generated [--splice-report behavior_splice.json]` checks a generated tree (it does
  not write COUNTS.txt): the declaration and its UFUNCTION specifier (BlueprintCallable for getter/exec,
  WithValidation for the RPC of a validate), the definition with the same signature and const, and that the body
  equals the `.inc`. An entry the splice report rejects is counted as `not spliced`, not as a failure;
- `.inc` exists; CONFIDENCE tag matches the manifest; original address present; no TODO/FIXME/TBD/XXX/
  placeholder/"not recovered"/"..." text (checked on the raw text, comments included);
- best-effort token check: each bare identifier and each `this->X` in the body must be a parameter, a local,
  a type (generated header or file), a function, a member of the class or a generated ancestor, or an engine
  name from the list above. A local is a declaration that starts a statement, a block or a `for` head, and its name
  may follow `*` or `&` (ninth pass; a template-typed local is not recognised and needs a plain typed name instead).
  The names of the class's member fields and helpers are accepted. `obj.X`, `obj->X` and `::`-qualified names are not
  checked. Numeric literal suffixes (`0.0f`) are skipped;
- skipped entries have a reason and no `inc_path`.

Limits: the token check is textual. It does not check types or the decompile-to-C++ meaning; that is the review's job.

## How the bodies were checked

- Folded stubs: the three shared addresses were disassembled from the runtime dump (`mov al,1; ret`, `ret 0`,
  `xor al,al; ret`). The name of each vtable slot comes from the RPC thunk naming (the slot's `_Validate` or
  `_Implementation`), which is the medium part; the body itself is high.
- Bits: `bool_bits.py` gives (byte, mask) for each bool. A getter reads a bit only if its raw decompile masks that
  byte with one bit and the pair maps to exactly one bool in the class chain.
- Whole-byte bool reads are accepted only where the byte holds one bool written by a whole-byte `mov` (medium).

## Screen of all 1,130 records (`porting/generated/decomp/portability.json`)

| Result | Count |
|---|---:|
| ported (this manifest) | 167 |
| verdict `not usable` (not screened further) | 210 |
| `unnamed_call` (calls an unidentified `FUN_*`; 15 of these call only identified helpers and FFrame fetches) | 379 |
| `rpc_exec_thunk` (exec thunk; the native bodies are the impl/validate records) | 164 |
| `vtable_call` (calls an unnamed vtable slot) | 113 |
| `folded_identical_body` (shared address, not ported by this rule) | 55 |
| `whole_or_multibit_byte_read` | 10 |
| `unmapped_offset` | 12 |
| `global_constant` (reads a data constant with no recovered value) | 5 |
| `jump_table_or_thunk` | 2 |
| `ambiguous_offset` | 1 |
| `other` | 12 |

Counts are for the screen rule in `skips.py`, applied to the 1,130 records. The verdicts were recomputed
only for the ported and skipped entries.

Dashboard function coverage uses `COUNTS.txt`'s `ported_functions` value: ported manifest entries excluding `member_field` storage declarations. The legacy `ported` total remains an entry count for checker reporting; `member_fields` records supporting storage separately. Neither declaration nor source-body coverage establishes runtime parity.

For a recovered internal reflected function, `reflection_flags` may pin the original numeric mask. Generation rejects a mismatch; the class policy must preserve original callable/access metadata, and the checker verifies it. This permits protected non-Blueprint-callable native callbacks without adding a new Blueprint API. `GameState` is the inherited AGameModeBase property used by the original Athena callback.

Reflected `getter`/`exec` snippets may carry a boolean `virtual` when original
vtable/exec evidence establishes native virtual dispatch. It is independent of
reflection flags. The generator preserves the qualifier in declarations and its
declaration guard; static functions, RPCs and Blueprint events cannot opt into
this path. Keep signatures free of linkage words and record original slot/table
evidence in the source batch. Existing entries without this key retain their
previous declarations.

### Blueprint native event implementations

Use `native_event_impl` only for an opted-in reflected native event with a recovered virtual implementation. The `function` and reflection-derived `signature` name the `_Implementation` method; `reflection_flags` is required and pins the original mask. The generator rejects mismatched flags, signatures, RPCs, static functions, delegates, and RepNotify targets. It emits the reflected `BlueprintNativeEvent` declaration with the original callable, pure, cosmetic, authority, exec, const and access metadata, followed by a virtual `_Implementation` declaration. Only the implementation receives the snippet; UHT generates the event dispatcher. An ordinary `getter` or `exec` cannot opt an event in.

Member helpers may set `"static": true` with an ordinary signature (without the static keyword). The generator emits a static class declaration and ordinary qualified definition. Static and virtual are mutually exclusive; static helpers cannot be const. This differs from kind `static_helper`, which remains a private translation-unit free function.
