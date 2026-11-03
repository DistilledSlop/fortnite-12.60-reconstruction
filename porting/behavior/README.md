# Behaviour snippets (porting/behavior)

Gameplay source update (2026-11-02): six additional bodies and one native
controller delegate restore stagger/respawn/parachute queries, squad preservation
and team-kill notification. See `pawn_state_evidence.md` for original field/mask
proofs and `../HANDOFF.md` for validation. Scratch generation accepts259 entries
without rejects; the four affected gameplay classes verify224/224 entries and
pass UE C++ syntax checks. Shared Native promotion, fresh UHT, linking and runtime
validation remain pending. Historical counts/checkpoints below are unchanged.

Hand-ported native function bodies, taken from the decompiled output of the shipping client
(`porting/generated/decomp/` and `porting/generated/decomp_improved/`, both git-ignored). Each `.inc`
is our own C++ re-expression of what the decompile does, not a copy of it.

Status (see `MANIFEST.json`, 439 entries): **251 ported** (34 getters, 29 onrep, 61 impl, 25 exec, 73 validate, 4 helper, 1 static_helper, 19 member_field, 5 struct_net; confidence 211 high, 40 medium), **188 skipped** with a reason (of which 17 are callees from the queue). `python3 -I porting/behavior/check_behavior.py` reports 0 failures; `COUNTS.txt` has 244 signatures declared in the previously generated header and 6 pending declarations. The regenerated tree separately verified all 251 splices with no failures.

Eleventh pass, the native NetSerialize bodies (see `porting/decompilation_notes.md`, "Eleventh pass"): five structs get their
recovered `NetSerialize` through the new kind `struct_net` (below): `FSharedRepMovement`, `FFortAISharedRepMovement`,
`FReplicatedPhysicsPawnState`, `FFortAIAttributeReplicationProxy` and `FCreateBuildingActorData` (the last one MEDIUM on the
saving yaw wrap). `FAthenaPawnReplayData` stays on its fallback (its wire depends on runtime state and key handling).

Twelfth pass (see `porting/decompilation_notes.md`, "Twelfth pass"): no new entries; at that point there were 437 entries, 249 ported and 188 skipped. One probe run over 18 callees of the five bodies that the tenth pass found blocked (`ServerClearSkydiveLeader_Implementation`, `OnClientVolumeManagerReplicated`, `Server_SetCanEditCreativeIsland_Implementation`, `ClientSendMatchStatsForPlayer_Implementation`, `ClientNotifyTeamLost_Implementation`). The identities are recorded in the notes and the queue, and none of the five bodies is ported: each still has an unnamed member, virtual slot or large callee (the notes give the exact blocker). The scratch promote splices 249 entries with 0 rejected, and `check_behavior.py --gen-root` on that tree reports FAIL 0.

Tenth pass (see `porting/decompilation_notes.md`, "Tenth pass"): the four rank-2 callees of `CALLEE_PORT_QUEUE.md` are identified (one small probe run) and the stat-kill bodies that use them are ported: `OnRep_Kills` (onrep), `OnRep_TotalPlayerScore` (onrep, one tail call), `OnRep_TeamKillScore` (onrep, its callee `0x1419e4f10` identified from two instructions) and `OnRep_AIKills` (helper, protected: it is not a RepNotify, so the generator declares it). Three new `member_field` entries are the no-argument delegates of the controllers (`AFortPlayerControllerZone` at `+0x28f0` and `+0x2908`, `AFortPlayerControllerAthena` at `+0x3210`), with placeholder names. `OnRep_DeathInfo` stays skipped (75 unnamed callees). The rank-3 scope call `0x142cd3770` is a no-op. `GetOwner` was added to `ENGINE_NAMES` (checker only).

Third pass (35 new bodies, see `porting/decompilation_notes.md`, "Third pass"): 15 empty bodies (exec thunks that
tail-jump to the shared `ret` at `0x1406801c0`), 14 bodies that broadcast a multicast delegate through the dispatcher
`0x1406a9dc0` (identified as `ProcessMulticastDelegate`), 4 FVector accessors, a float reset, and one RPC
validate. Held back as uncertain: six validates on the folded body `0x140f6b550` (identity rests on vtable slot names),
and the RPC validates that share folded bodies with mismatched argument registers.

Ninth pass (see `porting/decompilation_notes.md`, "Ninth pass"): 28 new entries. 16 `member_field` (eleven int32 point caches, the four native score delegate lists and the `AFortTeamInfo` int32), 2 `helper` (`UpdateScoreForEvent`, `UpdateTotalPlayerScore`), 1 `static_helper` (`GetFortGameStateAthena`) and 9 `onrep` stat bodies (`OnRep_Num*`, two of them with the profiling-scope call `0x142cd3770` ported as a no-op). The Eighth pass helper `GetActivePlaylist` is renamed `GetCurrentPlaylistAthena` in place (no second entry). Each new snippet was spliced and verified in a scratch promote run (237 spliced, 0 rejected) and syntax-checked with the UE 4.25 clang (see the decompilation notes for the scope of that check).

Sixth pass (see `porting/decompilation_notes.md`, "Sixth pass"): nine bodies. Six `ClientBroadcastOn*Finished_Implementation` (the eight missing `InlineMulticastDelegate` members are emitted now; their signatures are in declaration order), the zipline implementation and validate, and `ClientReceiveSquadQuickChatMessage_Implementation`. Each was spliced and verified in a scratch promote run (208 spliced, 0 rejected).

Frontend prerequisite: `UFortRuntimeOptions::GetRuntimeOptions` is a reflected static BlueprintPure
function (`0x14022401`). Its exec thunk at `0x142ace3c0` calls `0x1426fdde0`, whose
class-default-object lookup returns the `UFortRuntimeOptions` default object. The snippet uses
`::GetMutableDefault<UFortRuntimeOptions>()` for the same mutable pointer. Recover the two bodies
with `probe_callees.py` at those addresses; the original bytes contain no additional instance
field access. This evidence establishes the getter's body, not frontend initialization or a load.
`GetContext` still depends on unresolved context lookups; `GetGameVersion` derives its result
from runtime branch and version information and is not replaced by a constant.

Frontend candidate `UFortRuntimeOptions::GetGameVersion`: the exec at `0x142acdef0`
calls `0x1426ff110`. The latter uses the build-version and branch strings (`0x142ba33f0`,
`0x142ba33d0`), extracts after the last `+` and before `-CL-`, removes `Release-`
case-insensitively, falls back to the branch if the result is empty, and appends the
current engine changelist. The `UE4` branch instead starts with `Dev Build `. The
`0x142c96130`/`0x142c88340` current-version construction and `0x142c96f60` changelist
mask match `FEngineVersion::Current().GetChangelist()`; UE 4.25 `FApp` wraps the two
build-settings getters. The snippet calls those runtime APIs rather than hardcoding the
supplied build's values. `FortniteLinux` compiled (`generated/frontend-probes/build-getgameversion.log`),
and an opt-in runtime probe returned `v4.25 (0)` for the local `++UE4+Release-4.25-CL-0`
build (`generated/frontend-probes/probe-getgameversion.log`). This does not establish the
original client's display string or an interactive frontend.
Decompiles and disassembly are in ignored `generated/frontend-native-evidence/`:
`fortnite-frontend-{thunks,bodies}/` and `version-dependencies/`. To repeat the new
probes, run `porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input INPUT.json
--out porting/generated/frontend-native-evidence/version-dependencies --timeout 45 --mem 12G`
with `INPUT.json` a list of addresses `0x142ba33d0`, `0x142ba33f0`, `0x142c88340`,
`0x142c96130`, `0x142bace90`, and `0x142bb18c0`. The probe rolls back its transaction.

The next frontend helper probe is in this worktree's ignored
`generated/frontend-next-{thunks,bodies}/`. `UFortGlobalUIContext::QuitGame`
exec `0x14524a9c0` jumps to `0x144ce3b10`; its 341-byte body checks object
types and calls seven unidentified or low-level functions plus one virtual
slot. `SetInputMode` exec `0x14524b770` calls `0x144ce7b00` (284 bytes,
three direct calls and five indirect calls). `UFortFrontEndContext::SetFrontEndCamera`
exec `0x14523cf30` calls `0x144ce6fd0` (1,986 bytes, 36 direct calls and four
indirect calls). `UFortBuildWatermark::UpdateWatermark` exec `0x14520a5a0`
jumps to `0x144e235f0` (593 bytes, 12 direct and five indirect calls, including
widget text updates through unresolved fields). The native bodies cannot yet
be translated faithfully into the generated classes; no snippets were added
for these four functions. `BlueprintContextLibrary::GetContext` remains blocked
on its context-manager lookup and unknown fields.

Additional frontend helper probes (2026-11-01) are preserved under ignored
`generated/frontend-native-evidence/{ui-helper-candidates,ui-helper-bodies,execute-client-event-deps,get-frontend-camera-deps,get-context-deps}/`.
Each directory has raw `.c`, `.asm`, `.json`, and `index.json` output from
`ghidra/probe_callees.py`; the corresponding input address lists are in the
parent evidence directory. No new snippet was ported from these probes:

- `UFortKismetLibrary::ExecuteClientEvent` exec `0x14296a4b0` calls body
  `0x141eece20`. The body resolves a world context with `0x141e5b2c0` and
  forwards four event arguments to `0x141e59c80`, or logs `No WorldContext!`.
  The dispatcher is 1,216 bytes and builds `HandleClientEvent_%s` function
  names, searches an event registry, and invokes a virtual slot. Its registry
  and argument types have not been mapped to generated classes. A generic
  Blueprint event call would not preserve the observed filtering and fallback.
- `UFortFrontEndContext::GetFrontEndCamera` exec `0x14523a750` calls
  `0x144ccc6f0`. That body obtains an object through `0x144cb7c20`, follows
  another object through `0x141ca78b0`, and reads byte `+0xba`
  through `0x1412b1090`; it returns zero when either lookup fails. The
  concrete object and byte field have not been identified. This getter is a
  dependency clue for `SetFrontEndCamera`, not a safely portable body yet.
- `UBlueprintContextLibrary::GetContext` exec `0x140823580` calls
  `0x140822310`. Its 751-byte lookup `0x140822490` handles local-player and
  game-instance subsystems, then a separate context registry at manager `+0x20`;
  `0x140822100` initializes a shared singleton. Seven original class literals now
  prove the widget/controller/local-player routing and subsystem types; see
  [context-lookup-evidence.md](../frontend/context-lookup-evidence.md). Registry
  construction and lifecycle remain unfinished; no helper body was ported.

Reproduce these bounded probes with `porting/.venv/bin/python -I
porting/ghidra/probe_callees.py --input
porting/generated/frontend-native-evidence/<input>.json --out
porting/generated/frontend-native-evidence/<output> --timeout 35 --mem 12G`.
The five input basenames match the five output directories above. The script rolled
back its program transaction, though Ghidra project index/metadata files
changed during the read-only run. These probes alone do not validate UI class
loading or display.

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
