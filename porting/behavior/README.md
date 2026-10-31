# Behaviour snippets (porting/behavior)

Hand-ported native function bodies, taken from the decompiled output of the shipping client
(`porting/generated/decomp/` and `porting/generated/decomp_improved/`, both git-ignored). Each `.inc`
is our own C++ re-expression of what the decompile does, not a copy of it.

Status (see `MANIFEST.json`, 395 entries): **199 ported** (32 getters, 17 onrep, 53 impl, 25 exec, 72 validate; confidence 182 high, 17 medium), **196 skipped** with a reason (of which 17 are callees from the queue). Of the 199, 70 are declared in the generated headers; 129 need a declaration that the generator does not emit yet. `python3 porting/behavior/check_behavior.py` reports 0 failures and 3 pending declarations (`AFortPlayerPawnAthena::SetDamageCameraShakeClass`, `AFortPlayerPawn::GetPreviousPosition`, `AFortPlayerPawn::GetPreviousVelocity`). Three RPC entries that an earlier generator run declared (`ServerSendZiplineState_Implementation`, `ServerSendZiplineState_Validate`, `ClientReceiveSquadQuickChatMessage_Implementation`) are now skipped as not declared: their parameter structs (`FZiplinePawnState`, `FAthenaQuickChatActiveEntry`) are not generated.

Third pass (35 new bodies, see `porting/decompilation_notes.md`, "Third pass"): 15 empty bodies (exec thunks that
tail-jump to the shared `ret` at `0x1406801c0`), 14 bodies that broadcast a multicast delegate through the dispatcher
`0x1406a9dc0` (identified as `ProcessMulticastDelegate`), 4 FVector accessors, a float reset, and one RPC
validate. Held back as uncertain: six validates on the folded body `0x140f6b550` (identity rests on vtable slot names),
and the RPC validates that share folded bodies with mismatched argument registers.

## Files

- `MANIFEST.json`: array of entries `{class, function, kind, signature, signature_status, inc_path,
  confidence, address, status, proxy_score, decompile_source, notes}`. One entry per function, even when
  several functions share one body.
  - `status`: `ported` (has an `inc_path`) or `skipped` (`inc_path` is null, `notes` holds the reason).
  - `kind`: `getter`, `onrep` (RepNotify body), `impl` (RPC `_Implementation`), `validate` (RPC `_Validate`),
    `exec` (other BlueprintCallable body), `callee` (an unidentified helper queued in CALLEE_PORT_QUEUE.md).
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
  `FWeakObjectPtr`, `TWeakObjectPtr`, `FName`, `FString`, `FText`, `FVector`, `TArray`, `static_cast`, `Cast`.
- Each file starts with a `//` header comment: original address, decompile source, proxy score, what was
  inferred and `CONFIDENCE: HIGH|MEDIUM|LOW`. The checker requires the tag to match the manifest.

## Splice (implemented in gen_native_classes.py)

`gen_native_classes.py --promote` splices the ported entries (`status == "ported"`, 199 today). It is on by default.
Turn it off with `--no-behavior` or `FN_BEHAVIOR_SPLICE=0`; the output then equals the stub output. Per entry:

- `getter`, `exec` (`signature_status: proposed`): the non-RPC reflection function is emitted as
  `UFUNCTION(BlueprintCallable[, BlueprintPure])` (BlueprintPure when the reflection flag is set), with the manifest
  signature (`const` included) and the snippet as body. Only these two kinds add functions that are not RPCs.
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
  name from the list above. `obj.X`, `obj->X` and `::`-qualified names are not checked. Numeric literal
  suffixes (`0.0f`) are skipped;
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
