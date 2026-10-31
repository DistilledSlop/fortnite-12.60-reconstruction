# Behaviour snippets (porting/behavior)

Hand-ported native function bodies, taken from the decompiled output of the shipping client
(`porting/generated/decomp/` and `porting/generated/decomp_improved/`, both git-ignored). Each `.inc`
is our own C++ re-expression of what the decompile does, not a copy of it.

Status (see `MANIFEST.json`, 424 entries): **237 ported** (32 getters, 26 onrep, 61 impl, 25 exec, 73 validate, 3 helper, 1 static_helper, 16 member_field; confidence 204 high, 33 medium), **187 skipped** with a reason (of which 17 are callees from the queue). `python3 porting/behavior/check_behavior.py` reports 0 failures; `COUNTS.txt` has 217 signatures declared in the generated header and 19 pending declarations (the generator emits those when the splice is on: 16 member fields, 2 helpers and `GetCurrentPlaylistAthena`). The RPC entries that were skipped as not declared (the zipline implementation and validate, and the quick-chat implementation) are ported again.

Third pass (35 new bodies, see `porting/decompilation_notes.md`, "Third pass"): 15 empty bodies (exec thunks that
tail-jump to the shared `ret` at `0x1406801c0`), 14 bodies that broadcast a multicast delegate through the dispatcher
`0x1406a9dc0` (identified as `ProcessMulticastDelegate`), 4 FVector accessors, a float reset, and one RPC
validate. Held back as uncertain: six validates on the folded body `0x140f6b550` (identity rests on vtable slot names),
and the RPC validates that share folded bodies with mismatched argument registers.

Ninth pass (see `porting/decompilation_notes.md`, "Ninth pass"): 28 new entries. 16 `member_field` (eleven int32 point caches, the four native score delegate lists and the `AFortTeamInfo` int32), 2 `helper` (`UpdateScoreForEvent`, `UpdateTotalPlayerScore`), 1 `static_helper` (`GetFortGameStateAthena`) and 9 `onrep` stat bodies (`OnRep_Num*`, two of them with the profiling-scope call `0x142cd3770` ported as a no-op). The Eighth pass helper `GetActivePlaylist` is renamed `GetCurrentPlaylistAthena` in place (no second entry). Each new snippet was spliced and verified in a scratch promote run (237 spliced, 0 rejected) and syntax-checked with the UE 4.25 clang (see the decompilation notes for the scope of that check).

Sixth pass (see `porting/decompilation_notes.md`, "Sixth pass"): nine bodies. Six `ClientBroadcastOn*Finished_Implementation` (the eight missing `InlineMulticastDelegate` members are emitted now; their signatures are in declaration order), the zipline implementation and validate, and `ClientReceiveSquadQuickChatMessage_Implementation`. Each was spliced and verified in a scratch promote run (208 spliced, 0 rejected).

## Files

- `MANIFEST.json`: array of entries `{class, function, kind, signature, signature_status, inc_path,
  confidence, address, status, proxy_score, decompile_source, notes}`. One entry per function, even when
  several functions share one body.
  - `status`: `ported` (has an `inc_path`) or `skipped` (`inc_path` is null, `notes` holds the reason).
  - `kind`: `getter`, `onrep` (RepNotify body), `impl` (RPC `_Implementation`), `validate` (RPC `_Validate`),
    `exec` (other BlueprintCallable body), `callee` (an unidentified helper queued in CALLEE_PORT_QUEUE.md),
    `helper` (a non-reflected member function), `static_helper` (a static free function in the class .cpp) and
    `member_field` (a non-reflected data member); see "Helpers" and "Member fields" below.
  - `signature_status`: `declared` (copied verbatim from the generated header), `proposed` (not in the header;
    the generator must emit it), or `not_declared` (skipped entries).
- `<ClassName>/<FunctionName>.inc`: one snippet per ported function with its own header comment.
- `_shared/validate_return_true.inc` and `_shared/impl_empty.inc`: one body each for the folded stubs.
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
  The name is accepted by the checker only; the header is still needed. An entry that is not a helper or a field has no
  `includes`, so a body of an `impl`, `onrep`, `exec` or `getter` entry cannot bring one in.
- Each file starts with a `//` header comment: original address, decompile source, proxy score, what was
  inferred and `CONFIDENCE: HIGH|MEDIUM|LOW`. The checker requires the tag to match the manifest.

## Splice (implemented in gen_native_classes.py)

`gen_native_classes.py --promote` splices the ported entries (`status == "ported"`, 199 today). It is on by default.
Turn it off with `--no-behavior` or `FN_BEHAVIOR_SPLICE=0`; the output then equals the stub output. Per entry:

- `getter`, `exec` (`signature_status: proposed`): the non-RPC reflection function is emitted as
  `UFUNCTION(BlueprintCallable[, BlueprintPure])` (BlueprintPure when the reflection flag is set), with the manifest
  signature (`const` included) and the snippet as body. Apart from the helpers (see "Helpers" below), only these two kinds add functions that are not RPCs.
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
